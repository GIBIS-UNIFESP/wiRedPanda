// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/MainWindow.h"

#include <algorithm>
#include <functional>
#include <utility>

#ifdef Q_OS_WASM
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include <QActionGroup>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QKeySequence>
#include <QLocale>
#include <QLoggingCategory>
#include <QMap>
#include <QMessageBox>
#include <QPixmapCache>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QTabBar>
#include <QUrl>

#ifdef Q_OS_MAC
#include <QSvgRenderer>
#endif

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/ExerciseTourResources.h"
#include "App/Core/InstallRelativePaths.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Exercise/ExerciseEngine.h"
#include "App/Exercise/ExerciseOverlay.h"
#include "App/IO/RecentFiles.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Tour/TourEngine.h"
#include "App/Tour/TourOverlay.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/ExportController.h"
#include "App/UI/ICController.h"
#include "App/UI/LanguageManager.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/MessageDialog.h"
#include "App/UI/SceneUiBinder.h"
#include "App/UI/UpdateController.h"
#include "App/UI/WorkspaceManager.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"

#ifdef USE_KDE_FRAMEWORKS
#include <KActionCollection>
#include <KStandardAction>
#include <KToolBar>
using namespace Qt::StringLiterals;
#endif

#ifdef Q_OS_MAC
void ensureSvgUsage() {
    QSvgRenderer dummy; // for macdeployqt to add libqsvg.dylib
}
#endif

#ifdef Q_OS_WASM
const char *MainWindow::onBeforeUnload(int /*eventType*/, const void * /*reserved*/, void *userData)
{
    static_cast<MainWindow *>(userData)->updateSettings();
    return nullptr;
}
#endif

MainWindow::MainWindow(const QString &fileName, QWidget *parent)
    : MainWindowBase(parent)
    , m_ui(std::make_unique<MainWindowUi>())
{
    qCDebug(zero) << "wiRedPanda Version = " APP_VERSION " OR " << AppVersion::current;
    m_ui->setupUi(this);
    qCDebug(zero) << "Settings fileName: " << Settings::fileName();

#ifdef USE_KDE_FRAMEWORKS
    setupKdeActions();
#endif

    // Must be created before setupLanguage/setupTheme since both may call palette methods.
    m_palette = new ElementPalette(m_ui.get(), this);

    // Shared IC-hover preview, owned by this MainWindow as a Qt child.
    m_icPreviewPopup = new ICPreviewPopup(this);

    m_exportController = new ExportController(*this, this);
    m_icController = new ICController(*this, this);
    m_binder = new SceneUiBinder(m_ui.get(), m_palette, m_icPreviewPopup, this, this);
    m_workspaceManager = new WorkspaceManager(m_ui->tab, *this, this);

    // The manager owns the tab model and announces active-tab changes; the shell
    // rebinds the chrome. The binder forwards scene-driven navigation back to the manager.
    connect(m_workspaceManager, &WorkspaceManager::currentTabChanged, this, &MainWindow::onCurrentTabChanged);
    connect(m_workspaceManager, &WorkspaceManager::titleChanged,      this, &MainWindow::updateWindowTitle);
    connect(m_binder, &SceneUiBinder::openICRequested, m_workspaceManager, &WorkspaceManager::openICInTab);
    connect(m_binder, &SceneUiBinder::loadFileRequested, m_workspaceManager, &WorkspaceManager::loadPandaFile);

    // Must be created before setupLanguage(): loading a translation can synchronously
    // emit translationChanged(), which retranslateUi() handles by calling isActive() here.
    m_exerciseEngine = new ExerciseEngine(this);
    m_tourEngine     = new TourEngine(this);

    setupLanguage();
    setupGeometry();
    setupTheme();

#ifdef Q_OS_WASM
    // On WASM, closeEvent may not fire when the browser tab is closed.
    // Register a beforeunload callback to persist window geometry.
    emscripten_set_beforeunload_callback(this, &MainWindow::onBeforeUnload);
#endif

    qCDebug(zero) << "Setting left side menus.";
    m_palette->populate();

    qCDebug(zero) << "Loading recent file list.";
    setupRecentFiles();

    qCDebug(zero) << "Setting connections";
    setupConnections();

    qCDebug(zero) << "Checking playing simulation.";
    // Start simulation running by default so the circuit is live on open.
    m_ui->actionPlay->setChecked(true);

    qCDebug(zero) << "Window title.";
    setWindowTitle("wiRedPanda " APP_VERSION);

    // Create shortcuts before the first tab so connectTab() can wire them up.
    setupShortcuts();

    qCDebug(zero) << "Building a new tab.";
    createNewTab();

    // Restore minimap visibility preference and apply to current tab if any. Position/size
    // are restored by WorkSpace itself (from Settings::minimapGeometry(), applied in its
    // first resizeEvent()) -- no push needed from here.
    m_ui->actionShowMinimap->setChecked(Settings::minimapVisible());
    if (currentTab()) currentTab()->setMinimapVisible(Settings::minimapVisible());

    qCDebug(zero) << "Opening file if not empty.";
    if (!fileName.isEmpty()) {
        loadPandaFile(fileName);
    }

    // 100 000 KB cache limit — large circuits with many IC pixmaps benefit from generous caching.
    QPixmapCache::setCacheLimit(100000);

    qCDebug(zero) << "Adding examples to menu";
    setupExamplesMenu();
    setupExercisesMenu();
    setupToursMenu();
}

#ifdef USE_KDE_FRAMEWORKS
void MainWindow::setupKdeActions()
{
    // Suppress "domain not set" warnings in test environments
    KLocalizedString::setApplicationDomain("wiredpanda");

    // KActionCollection::addAction() overwrites objectName with the collection key.
    // Set the legacy objectName AFTER addAction() so findChild("actionXxx") still works.
    auto addAction = [this](const QString &kdeName, const QString &legacyName,
                             const QString &text, const QString &iconPath,
                             const QKeySequence &shortcut = {}) -> QAction * {
        auto *a = new QAction(this);
        a->setText(text);
        if (!iconPath.isEmpty()) a->setIcon(QIcon(iconPath));
        actionCollection()->addAction(kdeName, a);
        a->setObjectName(legacyName);                       // override AFTER collection add
        if (!shortcut.isEmpty()) actionCollection()->setDefaultShortcut(a, shortcut);
        return a;
    };

    auto stdAction = [this](QAction *a, const QString &legacyName, const QString &collectionName) {
        a->setParent(this);
        actionCollection()->addAction(collectionName, a);
        a->setObjectName(legacyName);                       // override AFTER collection add
        return a;
    };

    // Standard actions without callbacks — setupConnections() wires all signals for both paths
    using Obj  = QObject *;
    using Slot = const char *;
    m_ui->actionNew       = stdAction(KStandardAction::openNew(  static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionNew"_s,       u"file_new"_s);
    m_ui->actionOpen      = stdAction(KStandardAction::open(     static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionOpen"_s,      u"file_open"_s);
    m_ui->actionSave      = stdAction(KStandardAction::save(     static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionSave"_s,      u"file_save"_s);
    m_ui->actionSaveAs    = stdAction(KStandardAction::saveAs(   static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionSaveAs"_s,    u"file_save_as"_s);
    m_ui->actionExit      = stdAction(KStandardAction::quit(     static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionExit"_s,      u"file_quit"_s);
    m_ui->actionCut       = stdAction(KStandardAction::cut(      static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionCut"_s,       u"edit_cut"_s);
    m_ui->actionCopy      = stdAction(KStandardAction::copy(     static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionCopy"_s,      u"edit_copy"_s);
    m_ui->actionPaste     = stdAction(KStandardAction::paste(    static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionPaste"_s,     u"edit_paste"_s);
    m_ui->actionSelectAll = stdAction(KStandardAction::selectAll(static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionSelectAll"_s, u"edit_select_all"_s);
    m_ui->actionZoomIn    = stdAction(KStandardAction::zoomIn(   static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionZoomIn"_s,    u"view_zoom_in"_s);
    m_ui->actionZoomOut   = stdAction(KStandardAction::zoomOut(  static_cast<Obj>(nullptr), static_cast<Slot>(nullptr), nullptr), u"actionZoomOut"_s,   u"view_zoom_out"_s);

    // ── Custom actions (created as window children, registered in collection) ───

    m_ui->actionReloadFile = addAction(u"wiredpanda_reload_file"_s, u"actionReloadFile"_s, i18n("Re&load File"),
        u":/Interface/Toolbar/reloadFile.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_F5));
    m_ui->actionDelete = addAction(u"wiredpanda_delete"_s, u"actionDelete"_s, i18n("&Delete"),
        u":/Interface/Toolbar/delete.svg"_s, QKeySequence::Delete);
    m_ui->actionRename = addAction(u"wiredpanda_rename"_s, u"actionRename"_s, i18n("&Rename"),
        u":/Interface/Toolbar/rename.svg"_s, QKeySequence(Qt::Key_F2));
    m_ui->actionChangeTrigger = addAction(u"wiredpanda_change_trigger"_s, u"actionChangeTrigger"_s, i18n("Cha&nge Trigger"),
        u":/Components/Input/buttonOff.svg"_s, QKeySequence(Qt::Key_F3));
    m_ui->actionRotateRight = addAction(u"wiredpanda_rotate_right"_s, u"actionRotateRight"_s, i18n("R&otate right"),
        u":/Interface/Toolbar/rotateR.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_R));
    m_ui->actionRotateLeft = addAction(u"wiredpanda_rotate_left"_s, u"actionRotateLeft"_s, i18n("Rotate &left"),
        u":/Interface/Toolbar/rotateL.svg"_s, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    m_ui->actionFlipHorizontally = addAction(u"wiredpanda_flip_h"_s, u"actionFlipHorizontally"_s, i18n("&Flip horizontally"),
        {}, QKeySequence(Qt::CTRL | Qt::Key_H));
    m_ui->actionFlipVertically = addAction(u"wiredpanda_flip_v"_s, u"actionFlipVertically"_s, i18n("Flip &vertically"), {});
    m_ui->actionClearSelection = addAction(u"wiredpanda_clear_selection"_s, u"actionClearSelection"_s, i18n("Cl&ear selection"),
        u":/Interface/Toolbar/clearSelection.svg"_s, QKeySequence(Qt::Key_Escape));
    m_ui->actionResetZoom = addAction(u"wiredpanda_reset_zoom"_s, u"actionResetZoom"_s, i18n("&Reset Zoom"),
        u":/Interface/Toolbar/zoomReset.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_0));
    m_ui->actionFastMode = addAction(u"wiredpanda_fast_mode"_s, u"actionFastMode"_s, i18n("&Fast Mode"),
        u":/Interface/Toolbar/fast.svg"_s);
    m_ui->actionFastMode->setCheckable(true);
    m_ui->actionGates = addAction(u"wiredpanda_gates"_s, u"actionGates"_s, i18n("&Gates"),
        u":/Components/Logic/nor.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_2));
    m_ui->actionGates->setCheckable(true);
    m_ui->actionGates->setChecked(true);
    m_ui->actionWires = addAction(u"wiredpanda_wires"_s, u"actionWires"_s, i18n("&Wires"),
        u":/Interface/Toolbar/wires.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_1));
    m_ui->actionWires->setCheckable(true);
    m_ui->actionWires->setChecked(true);
    m_ui->actionFullscreen = addAction(u"wiredpanda_fullscreen"_s, u"actionFullscreen"_s, i18n("F&ullscreen"),
        {}, QKeySequence(Qt::Key_F11));
    m_ui->actionLabelsUnderIcons = addAction(u"wiredpanda_labels_under_icons"_s, u"actionLabelsUnderIcons"_s, i18n("Labels under icons"), {});
    m_ui->actionLabelsUnderIcons->setCheckable(true);
    m_ui->actionICPreview = addAction(u"wiredpanda_ic_preview"_s, u"actionICPreview"_s, i18n("Show IC Preview"), {});
    m_ui->actionICPreview->setCheckable(true);
    m_ui->actionLightTheme = addAction(u"wiredpanda_theme_light"_s, u"actionLightTheme"_s, i18n("&Light"), {});
    m_ui->actionLightTheme->setCheckable(true);
    m_ui->actionLightTheme->setChecked(true);
    m_ui->actionDarkTheme = addAction(u"wiredpanda_theme_dark"_s, u"actionDarkTheme"_s, i18n("&Dark"), {});
    m_ui->actionDarkTheme->setCheckable(true);
    m_ui->actionSystemTheme = addAction(u"wiredpanda_theme_system"_s, u"actionSystemTheme"_s, i18n("&System"), {});
    m_ui->actionSystemTheme->setCheckable(true);
    m_ui->actionPlay = addAction(u"wiredpanda_play"_s, u"actionPlay"_s, i18n("&Play/Pause"),
        u":/Interface/Toolbar/play.svg"_s, QKeySequence(Qt::Key_F5));
    m_ui->actionPlay->setCheckable(true);
    {
        QIcon playIcon(u":/Interface/Toolbar/play.svg"_s);
        playIcon.addFile(u":/Interface/Toolbar/pause.svg"_s, QSize(), QIcon::Normal, QIcon::On);
        m_ui->actionPlay->setIcon(playIcon);
    }
    m_ui->actionRestart = addAction(u"wiredpanda_restart"_s, u"actionRestart"_s, i18n("&Restart"),
        u":/Interface/Toolbar/reset.svg"_s);
    m_ui->actionWaveform = addAction(u"wiredpanda_waveform"_s, u"actionWaveform"_s, i18n("&Waveform"),
        u":/Interface/Toolbar/dolphin_icon.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_W));
    m_ui->actionMute = addAction(u"wiredpanda_mute"_s, u"actionMute"_s, i18n("Mute"),
        u":/Components/Output/Buzzer/BuzzerOff.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_M));
    m_ui->actionMute->setCheckable(true);
    {
        QIcon muteIcon(u":/Components/Output/Buzzer/BuzzerOff.svg"_s);
        muteIcon.addFile(u":/Interface/Toolbar/mute.svg"_s, QSize(), QIcon::Normal, QIcon::On);
        m_ui->actionMute->setIcon(muteIcon);
    }
    m_ui->actionBackground_Simulation = addAction(u"wiredpanda_background_simulation"_s,
        u"actionBackground_Simulation"_s, i18n("Background Simulation"), {});
    m_ui->actionBackground_Simulation->setCheckable(true);
    m_ui->actionBackground_Simulation->setIcon(QIcon::fromTheme(u"media-playlist-repeat"_s));
    m_ui->actionExportToArduino = addAction(u"wiredpanda_export_arduino"_s, u"actionExportToArduino"_s,
        i18n("E&xport to Arduino"), u":/Interface/Toolbar/arduino.svg"_s, QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I));
    m_ui->actionExportToSystemVerilog = addAction(u"wiredpanda_export_systemverilog"_s, u"actionExportToSystemVerilog"_s,
        i18n("Export to &SystemVerilog"), u":/Interface/Toolbar/verilog.svg"_s, QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_O));
    m_ui->actionExportToPdf = addAction(u"wiredpanda_export_pdf"_s, u"actionExportToPdf"_s,
        i18n("Export to &PDF"), u":/Interface/Toolbar/pdf.svg"_s, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    m_ui->actionExportToImage = addAction(u"wiredpanda_export_image"_s, u"actionExportToImage"_s,
        i18n("Export to &Image"), u":/Interface/Toolbar/png.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_E));
    m_ui->actionMakeSelfContained = addAction(u"wiredpanda_make_self_contained"_s, u"actionMakeSelfContained"_s,
        i18n("Make file self-contained"), {});
    m_ui->actionAbout = addAction(u"wiredpanda_about"_s, u"actionAbout"_s,
        i18n("&About"), u":/Interface/Toolbar/help.svg"_s, QKeySequence(Qt::Key_F1));
    m_ui->actionAboutQt = addAction(u"wiredpanda_about_qt"_s, u"actionAboutQt"_s,
        i18n("About &Qt"), u":/Interface/Toolbar/helpQt.svg"_s);
    m_ui->actionAboutThisVersion = addAction(u"wiredpanda_about_this_version"_s, u"actionAboutThisVersion"_s,
        i18n("About this version"), {});
    m_ui->actionShortcutsAndTips = addAction(u"wiredpanda_shortcuts_tips"_s, u"actionShortcutsAndTips"_s,
        i18n("Shortcuts and Tips"), {});
    m_ui->actionShortcutsAndTips->setIcon(QIcon::fromTheme(u"help-about"_s));
    m_ui->actionReportTranslationError = addAction(u"wiredpanda_report_translation"_s, u"actionReportTranslationError"_s,
        i18n("Report Translation Error"), u":/Interface/Toolbar/help.svg"_s);

    // ── Dynamic menus (populated at runtime) ────────────────────────────────
    // Set objectName on each so tests using findChild<QMenu*>("menuXxx") work.
    m_ui->menuRecentFiles  = new QMenu(i18n("&Recent files:"), this);
    m_ui->menuRecentFiles->setObjectName(u"menuRecentFiles"_s);
    m_ui->menuRecentFiles->setEnabled(false);
    m_ui->menuRecentFiles->setIcon(QIcon(u":/Interface/Toolbar/recentFiles.svg"_s));
    m_ui->menuTheme        = new QMenu(i18n("&Theme"), this);
    m_ui->menuTheme->setObjectName(u"menuTheme"_s);
    m_ui->menuTheme->addAction(m_ui->actionSystemTheme);
    m_ui->menuTheme->addAction(m_ui->actionLightTheme);
    m_ui->menuTheme->addAction(m_ui->actionDarkTheme);
    m_ui->menuExamples     = new QMenu(i18n("Examples"), this);
    m_ui->menuExamples->setObjectName(u"menuExamples"_s);
    m_ui->menuLanguage     = new QMenu(i18n("&Language"), this);
    m_ui->menuLanguage->setObjectName(u"menuLanguage"_s);
    m_ui->menuTranslation  = new QMenu(i18n("&Help Translate"), this);
    m_ui->menuTranslation->setObjectName(u"menuTranslation"_s);
    m_ui->menuTranslation->addAction(m_ui->actionReportTranslationError);

    // KXmlGuiWindow auto-injects two action sets that clash with wiRedPanda's
    // shortcuts; both would trigger checkAmbiguousShortcuts() in createGUI() to
    // pop a modal KMessageBox that hangs headless tests.
    //   - KCommandBar  (open_kcommand_bar): Ctrl+Alt+I  vs  Export-to-Arduino
    //   - KHelpMenu    (help_contents):     F1          vs  About
    // wiRedPanda owns its own Help menu, so disable both.
    setCommandBarEnabled(false);
    setHelpMenuEnabled(false);

    // ── Load XML GUI and let KXmlGuiWindow build menus/toolbars ─────────────
    setupGUI(KXmlGuiWindow::Default, u":/wiredpanda/wiredpandaui.rc"_s);

    // ── Post-setupGUI: grab KDE-created menus and assign to m_ui pointers ───
    for (QAction *a : menuBar()->actions()) {
        QMenu *m = a->menu();
        if (!m) continue;
        const QString name = m->objectName();
        if (name == u"file"_s)       m_ui->menuFile       = m;
        else if (name == u"edit"_s)  m_ui->menuEdit       = m;
        else if (name == u"view"_s)  m_ui->menuView       = m;
        else if (name == u"simulation"_s) m_ui->menuSimulation = m;
        else if (name == u"help"_s)  m_ui->menuHelp       = m;
    }

    // Insert recent files submenu into the File menu (after Open, before save)
    if (m_ui->menuFile) {
        QAction *saveAction = actionCollection()->action(u"file_save"_s);
        m_ui->menuFile->insertMenu(saveAction, m_ui->menuRecentFiles);
        m_ui->menuFile->insertSeparator(saveAction);
    }

    // Add dynamic menus to the menu bar (before Help)
    QAction *helpMenuAction = m_ui->menuHelp ? m_ui->menuHelp->menuAction() : nullptr;
    menuBar()->insertMenu(helpMenuAction, m_ui->menuExamples);
    menuBar()->insertMenu(helpMenuAction, m_ui->menuLanguage);
    menuBar()->insertMenu(helpMenuAction, m_ui->menuTranslation);

    // Grab toolbar and status bar pointers so existing code can use them
    // KToolBar inherits QToolBar; explicit cast needed due to Qt's strict pointer rules
    m_ui->mainToolBar = qobject_cast<QToolBar *>(toolBar(u"mainToolBar"_s));
    m_ui->statusBar   = statusBar();
}
#endif // USE_KDE_FRAMEWORKS

void MainWindow::setupLanguage()
{
    m_languageManager = new LanguageManager(this);
    connect(m_languageManager, &LanguageManager::translationChanged, this, &MainWindow::retranslateUi);

    QString language = Settings::language();
    if (language.isEmpty()) {
        const QLocale systemLocale  = QLocale::system();
        const QString systemLang   = systemLocale.name();
        const QString baseLang     = systemLang.split('_').first();
        qCDebug(zero) << "Auto-detected system locale:" << systemLang;

        const auto available = m_languageManager->availableLanguages();
        if (available.contains(systemLang)) {
            language = systemLang;
        } else if (available.contains(baseLang)) {
            language = baseLang;
        } else {
            qCDebug(zero) << "No translation for" << systemLang << "or" << baseLang << ", falling back to English";
            language = "en";
        }
        qCDebug(zero) << "Selected language:" << language;
    }

    m_languageManager->loadTranslation(language);
    populateLanguageMenu();
}

void MainWindow::setupGeometry()
{
    qCDebug(zero) << "Restoring geometry and setting zoom controls.";
    restoreGeometry(Settings::mainWindowGeometry());
    restoreState(Settings::mainWindowState());
    m_ui->splitter->restoreGeometry(Settings::splitterGeometry());
    m_ui->splitter->restoreState(Settings::splitterState());
}

void MainWindow::setupTheme()
{
    qCDebug(zero) << "Preparing theme and UI modes.";
    auto *themeGroup = new QActionGroup(this);
    for (auto *action : m_ui->menuTheme->actions()) {
        themeGroup->addAction(action);
    }
    themeGroup->setExclusive(true);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &MainWindow::updateTheme);
    updateTheme();
    setFastMode(Settings::fastMode());

    // Restore toolbar label style from previous session.
    m_ui->actionLabelsUnderIcons->setChecked(Settings::labelsUnderIcons());
    m_ui->mainToolBar->setToolButtonStyle(Settings::labelsUnderIcons() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);

    // Restore IC hover-preview visibility from previous session.
    m_ui->actionICPreview->setChecked(!Settings::icPreviewDisabled());
    m_ui->actionCheckForUpdates->setChecked(!Settings::updateChecksDisabled());
}

void MainWindow::setupRecentFiles()
{
    m_recentFiles = new RecentFiles(this);
    connect(m_workspaceManager, &WorkspaceManager::recentFileAdded, m_recentFiles, &RecentFiles::addRecentFile);
    connect(m_recentFiles, &RecentFiles::recentFilesUpdated,   this,          &MainWindow::updateRecentFileActions);
    createRecentFileActions();
}

void MainWindow::setupExamplesMenu()
{
    const QString examplesPath = InstallRelativePaths::resolve(QStringLiteral("Examples"));

    if (!examplesPath.isEmpty()) {
        const auto entryList = QDir(examplesPath).entryList({"*.panda"}, QDir::Files);

        for (const auto &entry : entryList) {
            // Show a prettified title ("display-4bits-counter.panda" -> "Display 4bits
            // Counter") but keep the real path in setData() so the lookup never depends on
            // the (translatable, prettified) label — same pattern as the Recent Files menu.
            QString title = QFileInfo(entry).completeBaseName();
            title.replace(QLatin1Char('-'), QLatin1Char(' '));
            title.replace(QLatin1Char('_'), QLatin1Char(' '));
            QStringList words = title.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            for (QString &word : words) {
                word[0] = word[0].toUpper();
            }

            auto *action = new QAction(words.join(QLatin1Char(' ')), this);
            action->setData(examplesPath + "/" + entry);

            connect(action, &QAction::triggered, this, [this] {
                if (auto *senderAction = qobject_cast<QAction *>(sender())) {
                    loadPandaFile(senderAction->data().toString());
                }
            });

            m_ui->menuExamples->addAction(action);
        }
    }

    if (m_ui->menuExamples->isEmpty()) {
        m_ui->menuExamples->menuAction()->setVisible(false);
    }
}

void MainWindow::populateContentMenu(QMenu *menu, const QString &categoryKey,
                                      const QString &openFolderText,
                                      const QString &openFolderFailureText,
                                      const QStringList &completed,
                                      const std::function<void(const QString &)> &onSelect)
{
    menu->clear(); // deletes the QActions it owns (and their connections) — safe to rebuild every open

    auto *openFolderAction = new QAction(openFolderText, menu);
    connect(openFolderAction, &QAction::triggered, this, [this, categoryKey, openFolderFailureText] {
        const QString dir = ExerciseTourResources::preferredContentDir(categoryKey);
        if (dir.isEmpty()) {
            MessageDialog::warning(this, openFolderFailureText, i18n("Error"));
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });
    menu->addAction(openFolderAction);

    const QIcon checkIcon = style()->standardIcon(QStyle::SP_DialogApplyButton);
    const QIcon circleIcon = style()->standardIcon(QStyle::SP_ArrowRight);

    const auto entries = ExerciseTourResources::discover(categoryKey);
    if (!entries.isEmpty()) {
        menu->addSeparator(); // only meaningful as a divider when there's something below it to divide from
    }
    for (const ExerciseTourResourceEntry &entry : entries) {
        QString title = ExerciseTourResources::translate(entry.id + QStringLiteral(".title"), entry.title);
        title.replace(QLatin1Char('&'), QStringLiteral("&&")); // literal '&' would otherwise be swallowed as a mnemonic marker
        auto *action = new QAction(title, menu);
        action->setIcon(completed.contains(entry.id) ? checkIcon : circleIcon);
        action->setStatusTip(ExerciseTourResources::translate(entry.id + QStringLiteral(".description"), entry.description));
        connect(action, &QAction::triggered, this, [onSelect, path = entry.path] { onSelect(path); });
        menu->addAction(action);
    }
}

void MainWindow::setupExercisesMenu()
{
    connect(m_ui->menuExercises, &QMenu::aboutToShow, this, [this] {
        populateContentMenu(m_ui->menuExercises, "Exercises",
            i18n("Open My Exercises Folder"),
            i18n("Could not create or access a folder for custom exercises."),
            Settings::completedExercises(),
            [this](const QString &path) { startExercise(path); });
    });
}

void MainWindow::setupToursMenu()
{
    connect(m_ui->menuTours, &QMenu::aboutToShow, this, [this] {
        populateContentMenu(m_ui->menuTours, "Tours",
            i18n("Open My Tours Folder"),
            i18n("Could not create or access a folder for custom tours."),
            Settings::completedTours(),
            [this](const QString &path) { startTour(path); });
    });
}

void MainWindow::setupShortcuts()
{
    // The scene-property shortcuts ( [ ] { } < > ) are owned by SceneUiBinder, which
    // re-targets them to the active tab's scene on each switch.
    auto *searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, m_ui->lineEditSearch, qOverload<>(&QWidget::setFocus));
}

void MainWindow::setupConnections()
{
    connect(m_ui->tab, &QTabWidget::currentChanged,    m_workspaceManager, &WorkspaceManager::onCurrentIndexChanged);
    connect(m_ui->tab, &QTabWidget::tabCloseRequested, m_workspaceManager, &WorkspaceManager::closeTab);

    connect(m_ui->actionAbout,                 &QAction::triggered,       this,                &MainWindow::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,               &QAction::triggered,       this,                &MainWindow::on_actionAboutQt_triggered);
    connect(m_ui->actionAboutThisVersion,      &QAction::triggered,       this,                &MainWindow::aboutThisVersion);
    connect(m_ui->actionReportTranslationError,&QAction::triggered,       this,                &MainWindow::on_actionReportTranslationError_triggered);
    connect(m_ui->actionChangeTrigger,         &QAction::triggered,       m_ui->elementEditor, &ElementEditor::changeTriggerAction);
    connect(m_ui->actionDarkTheme,             &QAction::triggered,       this,                &MainWindow::on_actionDarkTheme_triggered);
    connect(m_ui->actionSystemTheme,           &QAction::triggered,       this,                &MainWindow::on_actionSystemTheme_triggered);
    connect(m_ui->actionExit,                  &QAction::triggered,       this,                &MainWindow::on_actionExit_triggered);
    connect(m_ui->actionExportToArduino,       &QAction::triggered,       m_exportController,  &ExportController::exportArduinoDialog);
    connect(m_ui->actionExportToSystemVerilog, &QAction::triggered,       m_exportController,  &ExportController::exportSystemVerilogDialog);
    connect(m_ui->actionExportToImage,         &QAction::triggered,       m_exportController,  &ExportController::exportImageDialog);
    connect(m_ui->actionExportToPdf,           &QAction::triggered,       m_exportController,  &ExportController::exportPdfDialog);
    connect(m_ui->actionFastMode,              &QAction::triggered,       this,                &MainWindow::on_actionFastMode_triggered);
    connect(m_ui->actionFlipHorizontally,      &QAction::triggered,       this,                &MainWindow::on_actionFlipHorizontally_triggered);
    connect(m_ui->actionFlipVertically,        &QAction::triggered,       this,                &MainWindow::on_actionFlipVertically_triggered);
    connect(m_ui->actionAlignLeft, &QAction::triggered, this, &MainWindow::on_actionAlignLeft_triggered);
    connect(m_ui->actionAlignRight, &QAction::triggered, this, &MainWindow::on_actionAlignRight_triggered);
    connect(m_ui->actionAlignTop, &QAction::triggered, this, &MainWindow::on_actionAlignTop_triggered);
    connect(m_ui->actionAlignBottom, &QAction::triggered, this, &MainWindow::on_actionAlignBottom_triggered);
    connect(m_ui->actionAlignHorizontalCenter, &QAction::triggered, this, &MainWindow::on_actionAlignHorizontalCenter_triggered);
    connect(m_ui->actionAlignVerticalCenter, &QAction::triggered, this, &MainWindow::on_actionAlignVerticalCenter_triggered);
    connect(m_ui->actionDistributeHorizontally, &QAction::triggered, this, &MainWindow::on_actionDistributeHorizontally_triggered);
    connect(m_ui->actionDistributeVertically, &QAction::triggered, this, &MainWindow::on_actionDistributeVertically_triggered);
    connect(m_ui->actionFullscreen,            &QAction::triggered,       this,                &MainWindow::on_actionFullscreen_triggered);
    connect(m_ui->actionGates,                 &QAction::triggered,       this,                &MainWindow::on_actionGates_triggered);
    connect(m_ui->actionLabelsUnderIcons,      &QAction::triggered,       this,                &MainWindow::on_actionLabelsUnderIcons_triggered);
    connect(m_ui->actionICPreview,             &QAction::triggered,       this,                &MainWindow::on_actionICPreview_triggered);
    connect(m_ui->actionCheckForUpdates,       &QAction::triggered,       this,                &MainWindow::on_actionCheckForUpdates_triggered);
    connect(m_ui->actionShowMinimap,           &QAction::triggered,       this,                &MainWindow::on_actionShowMinimap_triggered);
    connect(m_ui->actionLightTheme,            &QAction::triggered,       this,                &MainWindow::on_actionLightTheme_triggered);
    connect(m_ui->actionMute,                  &QAction::triggered,       this,                &MainWindow::on_actionMute_triggered);
    connect(m_ui->actionNew,                   &QAction::triggered,       m_workspaceManager,  &WorkspaceManager::newTab);
    connect(m_ui->actionOpen,                  &QAction::triggered,       m_workspaceManager,  &WorkspaceManager::openFile);
    connect(m_ui->actionPlay,                  &QAction::toggled,         this,                &MainWindow::on_actionPlay_toggled);
    connect(m_ui->actionReloadFile,            &QAction::triggered,       m_workspaceManager,  &WorkspaceManager::reloadFile);
    connect(m_ui->actionRename,                &QAction::triggered,       m_ui->elementEditor, &ElementEditor::renameAction);

    // ElementEditor IC sub-circuit actions
    connect(m_ui->elementEditor, &ElementEditor::editSubcircuitRequested, this, [this](const QString &blobName, int icElementId) {
        if (currentTab()) {
            openICInTab(blobName, icElementId, currentTab()->scene()->icRegistry()->blob(blobName));
        }
    });
    connect(m_ui->elementEditor, &ElementEditor::openSubcircuitFileRequested, this, &MainWindow::loadPandaFile);
    connect(m_ui->elementEditor, &ElementEditor::embedSubcircuitRequested, m_icController, &ICController::embedSelectedIC);
    connect(m_ui->elementEditor, &ElementEditor::extractToFileRequested, m_icController, &ICController::extractSelectedIC);
    connect(m_ui->actionResetZoom,             &QAction::triggered,       this,                &MainWindow::on_actionResetZoom_triggered);
    connect(m_ui->actionZoomToFit,             &QAction::triggered,       this,                &MainWindow::on_actionZoomToFit_triggered);
    connect(m_ui->actionRestart,               &QAction::triggered,       this,                &MainWindow::on_actionRestart_triggered);
    connect(m_ui->actionRotateLeft,            &QAction::triggered,       this,                &MainWindow::on_actionRotateLeft_triggered);
    connect(m_ui->actionRotateRight,           &QAction::triggered,       this,                &MainWindow::on_actionRotateRight_triggered);
    connect(m_ui->actionSave,                  &QAction::triggered,       m_workspaceManager,  &WorkspaceManager::saveFile);
    connect(m_ui->actionSaveAs,               &QAction::triggered,       m_workspaceManager,  &WorkspaceManager::saveFileAs);
    connect(m_ui->actionSelectAll,             &QAction::triggered,       this,                &MainWindow::on_actionSelectAll_triggered);
    connect(m_ui->actionShortcutsAndTips,      &QAction::triggered,       this,                &MainWindow::on_actionShortcuts_and_Tips_triggered);
    connect(m_ui->actionWaveform,              &QAction::triggered,       this,                &MainWindow::on_actionWaveform_triggered);
    connect(m_ui->actionWires,                 &QAction::triggered,       this,                &MainWindow::on_actionWires_triggered);
    connect(m_ui->actionZoomIn,                &QAction::triggered,       this,                &MainWindow::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,               &QAction::triggered,       this,                &MainWindow::on_actionZoomOut_triggered);
    connect(m_palette,                         &ElementPalette::addElementRequested, this, [this](QMimeData *mimeData) {
        auto *tab = currentTab();
        if (!tab) {
            delete mimeData; // no scene to take ownership; don't leak the payload
            return;
        }
        // Land the element at the centre of what the user is currently viewing, not the
        // scene origin, so it's visible however the canvas is scrolled or zoomed.
        auto *view = tab->view();
        const QPointF center = view->mapToScene(view->viewport()->rect().center());
        tab->scene()->addItem(mimeData, center);
    });
    connect(m_ui->pushButtonAddIC,             &QPushButton::clicked,     m_icController,      &ICController::addICFromFile);
    connect(m_ui->pushButtonRemoveIC,          &QPushButton::clicked,     m_icController,      &ICController::showRemoveICHint);
    connect(m_ui->pushButtonRemoveIC,          &TrashButton::removeICFile,m_icController,      &ICController::removeICFile);
    connect(m_ui->pushButtonMakeSelfContained, &QPushButton::clicked,     m_icController,      &ICController::makeSelfContained);
    connect(m_ui->actionMakeSelfContained,     &QAction::triggered,       m_icController,      &ICController::makeSelfContained);

    // ICDropZone cross-section drag-and-drop
    connect(m_ui->dropZoneFileBased, &ICDropZone::extractByBlobNameRequested, m_icController, &ICController::extractICByBlobName);
    connect(m_ui->dropZoneEmbedded, &ICDropZone::embedByFileRequested, m_icController, &ICController::embedICByFile);

    // Embedded IC section buttons
    connect(m_ui->pushButtonAddEmbeddedIC, &QPushButton::clicked, m_icController, &ICController::addEmbeddedICFromFile);
    connect(m_ui->pushButtonRemoveEmbeddedIC, &QPushButton::clicked, m_icController, &ICController::showRemoveICHint);
    connect(m_ui->pushButtonRemoveEmbeddedIC, &TrashButton::removeEmbeddedIC, m_icController, &ICController::removeEmbeddedIC);

    // These edit actions always delegate to the current tab's scene, so they
    // never need to be rewired on tab switch.
    connectSceneAction(m_ui->actionClearSelection, &Scene::clearSelection);
    connectSceneAction(m_ui->actionCopy,           &Scene::copyAction);
    connectSceneAction(m_ui->actionCut,            &Scene::cutAction);
    connectSceneAction(m_ui->actionDelete,         &Scene::deleteAction);
    connectSceneAction(m_ui->actionPaste,          &Scene::pasteAction);
    connectSceneAction(m_ui->actionDuplicate,      &Scene::duplicateAction);
}

void MainWindow::connectSceneAction(QAction *action, void (Scene::*method)())
{
    connect(action, &QAction::triggered, this, [this, method] {
        if (currentTab()) {
            (currentTab()->scene()->*method)();
        }
    });
}

MainWindow::~MainWindow()
{
    // Tear down the active tab's chrome wiring before child objects are destroyed.
    m_binder->unbind();
}

void MainWindow::createNewTab()
{
    m_workspaceManager->createNewTab();
}

void MainWindow::setFastMode(const bool fastMode)
{
    m_ui->actionFastMode->setChecked(fastMode);

    if (currentTab()) {
        currentTab()->view()->setFastMode(fastMode);
    }
}

void MainWindow::on_actionExit_triggered()
{
    Application::guardedSlot(this, [this] {
        close();
    });
}

void MainWindow::save(const QString &fileName)
{
    m_workspaceManager->save(fileName);
}

void MainWindow::show()
{
    QMainWindow::show();

    qCDebug(zero) << "Checking for autosave file recovery.";
    m_workspaceManager->loadAutosaveFiles();

    auto *updateController = new UpdateController(this);
    updateController->checkForUpdates();

    // First-ever launch: auto-start the built-in UI walkthrough. Gated on interactiveMode so
    // it doesn't fire in the CLI/MCP-automation paths that also construct a MainWindow.
    if (Application::interactiveMode && !Settings::welcomeTourShown()) {
        Settings::setWelcomeTourShown(true);
        startTour(QStringLiteral(":/Tours/ui-overview.json"));
    }
}

void MainWindow::aboutThisVersion()
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.setWindowTitle("wiRedPanda " APP_VERSION);
    msgBox.setText(
        i18n("wiRedPanda %1\n\n"
           "This version includes automatic migration of older project files.\n"
           "When you open a project file older than the current version, it will be automatically "
           "upgraded to the current format and a versioned backup will be created.\n\n"
           "To open projects containing ICs (or boxes), appearances, and/or beWavedDolphin simulations, "
           "their files must be in the same directory as the main project file.\n"
           "wiRedPanda %1 will automatically list all other .panda files located "
           "in the same directory as the current project as ICs in the editor tab.\n"
           "You have to save new projects before accessing ICs and appearances, or running "
           "beWavedDolphin simulations.").arg(APP_VERSION));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Ok);

    msgBox.exec();
}

void MainWindow::on_actionWires_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Wires: %1").arg(checked));
        if (currentTab()) {
            currentTab()->scene()->showWires(checked);
        }
    });
}

void MainWindow::on_actionRotateRight_triggered()
{
    Application::guardedSlot(this, [this] {
        if (currentTab()) {
            currentTab()->scene()->rotateRight();
        }
    });
}

void MainWindow::on_actionRotateLeft_triggered()
{
    Application::guardedSlot(this, [this] {
        if (currentTab()) {
            currentTab()->scene()->rotateLeft();
        }
    });
}

void MainWindow::loadPandaFile(const QString &fileName)
{
    m_workspaceManager->loadPandaFile(fileName);
}

void MainWindow::openICInTab(const QString &blobName, int icElementId, const QByteArray &blob)
{
    m_workspaceManager->openICInTab(blobName, icElementId, blob);
}

void MainWindow::on_actionAbout_triggered()
{
    Application::guardedSlot(this, [this] {
        QMessageBox::about(
            this,
            "wiRedPanda",
            i18n("<p>wiRedPanda is software developed by students of the Federal University of São Paulo"
               " to help students learn about logic circuits.</p>"
               "<p>Software version: %1</p>"
               "<p><strong>Creators:</strong></p>"
               "<ul>"
               "<li> Davi Morales </li>"
               "<li> Lucas Lellis </li>"
               "<li> Rodrigo Torres </li>"
               "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
               "</ul>"
               "<p> wiRedPanda is currently maintained by Prof. Fábio Cappabianco, Ph.D., João Pedro M. Oliveira, Matheus R. Esteves and Maycon A. Santana.</p>"
               "<p> Please file a report at our GitHub page if you find a bug or want to request a new feature.</p>"
               "<p><a href=\"https://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
                .arg(QApplication::applicationVersion()));
    });
}

QString MainWindow::shortcutsHelpHtml() const
{
    // Keyed by label so rows sort alphabetically and any duplicate collapses.
    QMap<QString, QString> byLabel;

    // Undo/Redo belong to the active Scene's stack, not this window, so findChildren()
    // can't see them; add them explicitly with the platform-standard chords.
    byLabel.insert(i18n("Redo"), QKeySequence(QKeySequence::Redo).toString(QKeySequence::NativeText));
    byLabel.insert(i18n("Undo"), QKeySequence(QKeySequence::Undo).toString(QKeySequence::NativeText));

    const auto actions = findChildren<QAction *>();
    for (const auto *action : actions) {
        const QKeySequence seq = action->shortcut();
        if (seq.isEmpty()) {
            continue;
        }
        QString label = action->text();
        label.remove(QLatin1Char('&')); // strip menu mnemonics
        if (label.endsWith(QLatin1String("..."))) {
            label.chop(3);
        }
        label = label.trimmed();
        if (!label.isEmpty()) {
            byLabel.insert(label, seq.toString(QKeySequence::NativeText));
        }
    }

    QString rows;
    for (auto it = byLabel.cbegin(); it != byLabel.cend(); ++it) {
        rows += QStringLiteral("<tr><td><b>%1</b>&nbsp;&nbsp;&nbsp;</td><td>%2</td></tr>")
                    .arg(it.value().toHtmlEscaped(), it.key().toHtmlEscaped());
    }

    return i18n("<h1>Keyboard Shortcuts</h1>"
              "<table>%1</table>"
              "<h1>Element Property Navigation</h1>"
              "<ul style=\"list-style:none;\">"
              "<li> [ / ] : Previous / next primary property </li>"
              "<li> { / } : Previous / next secondary property </li>"
              "<li> &lt; / &gt; : Morph to previous / next element </li>"
              "</ul>"
              "<h1>General Tips</h1>"
              "<ul style=\"list-style:none;\">"
              "<li> Double-click a wire to create a node </li>"
              "<li> Drag an element from the left panel onto the canvas to add it </li>"
              "<li> Nudge the selection with the arrow keys (hold Shift for larger steps) </li>"
              "<li> Drop a .panda file onto the canvas to open it </li>"
              "</ul>",
        rows);
}

void MainWindow::on_actionShortcuts_and_Tips_triggered()
{
    Application::guardedSlot(this, [this] {
        MessageDialog::information(this, shortcutsHelpHtml(), i18n("Shortcuts and Tips"));
    });
}

void MainWindow::on_actionAboutQt_triggered()
{
    Application::guardedSlot(this, [this] {
        QMessageBox::aboutQt(this);
    });
}

void MainWindow::on_actionReportTranslationError_triggered()
{
    Application::guardedSlot(this, [] {
        QDesktopServices::openUrl(QUrl("https://hosted.weblate.org/projects/wiredpanda/wiredpanda"));
    });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool closeWindow = false;

    // If nothing is modified, ask once before exiting so the user can't
    // accidentally quit with a keyboard shortcut.  If there are unsaved changes,
    // delegate to closeFiles() which prompts per-tab.
    if (!m_workspaceManager->hasModifiedFiles()) {
        auto reply =
            QMessageBox::question(
                this,
                i18n("Exit") + " " + QApplication::applicationName(),
                i18n("Are you sure?"),
                QMessageBox::Cancel | QMessageBox::Yes,
                QMessageBox::Yes);

        if (reply == QMessageBox::Yes) {
            closeWindow = true;
        }
    } else if (m_workspaceManager->closeFiles()) {
        closeWindow = true;
    }

    if (closeWindow) {
        // Persist window/splitter layout so the next session opens the same way.
        updateSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::updateSettings()
{
    Settings::setMainWindowGeometry(saveGeometry());
    Settings::setMainWindowState(saveState());
    Settings::setSplitterGeometry(m_ui->splitter->saveGeometry());
    Settings::setSplitterState(m_ui->splitter->saveState());
}

void MainWindow::setICButtonsVisible(bool visible)
{
    m_ui->pushButtonAddIC->setVisible(visible);
    m_ui->pushButtonRemoveIC->setVisible(visible);
    m_ui->pushButtonMakeSelfContained->setVisible(visible);
}

void MainWindow::refreshICButtonsEnabled()
{
    // Add IC needs a real project directory to copy the chosen .panda into;
    // gating click-ability on a saved file avoids the "Save file first."
    // throw → modal-error UX dead end.
    const bool hasFile = currentTab() && currentTab()->fileInfo().isReadable();
    m_ui->pushButtonAddIC->setEnabled(hasFile);
}

QFileInfo MainWindow::currentFile() const
{
    return m_workspaceManager->currentFile();
}

bool MainWindow::hasModifiedFiles()
{
    return m_workspaceManager->hasModifiedFiles();
}

QDir MainWindow::currentDir() const
{
    return m_workspaceManager->currentDir();
}

QWidget *MainWindow::widget()
{
    return this;
}

DolphinHost *MainWindow::dolphinHost()
{
    return this;
}

ElementPalette *MainWindow::palette() const
{
    return m_palette;
}

void MainWindow::requestSave()
{
    m_workspaceManager->saveFile();
}

void MainWindow::showStatusMessage(const QString &message, int timeout)
{
    m_ui->statusBar->showMessage(message, timeout);
}

QFileInfo MainWindow::icListFile() const
{
    return m_workspaceManager->icListFile();
}

void MainWindow::on_actionSelectAll_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ui", QStringLiteral("Select all"));
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->selectAll();
    });
}

WorkSpace *MainWindow::currentTab() const
{
    return m_workspaceManager->currentTab();
}

void MainWindow::onCurrentTabChanged(WorkSpace *newTab)
{
    // Reaction to WorkspaceManager::currentTabChanged: rebind the shared chrome to the
    // new scene and refresh the tab-navigation view state (file-based IC list, buttons).
    // currentTab() already reflects newTab here (WorkspaceManager updates its current-tab
    // field before emitting this signal), so the tab actually being left is tracked
    // separately in m_previousTab rather than read via currentTab().
    WorkSpace *prevTab = m_previousTab;
    m_previousTab = newTab;

    m_binder->unbind(); // tear down the previously bound tab's chrome wiring
    // Hide the editor panel during the transition; SceneUiBinder::bind restores it
    // once the new scene's selection is known.
    m_ui->elementEditor->hide();

    // Detach exercise overlay from the leaving tab
    if (prevTab) {
        prevTab->setExerciseOverlay(nullptr);
    }
    if (m_exerciseOverlay && m_exerciseEngine && m_exerciseEngine->isActive()) {
        m_exerciseOverlay->hide();
        m_exerciseOverlay->setParent(nullptr);
    }

    if (!newTab) {
        // All tabs were closed; reset state.
        m_palette->updateICList(QFileInfo());
        m_palette->updateEmbeddedICList(nullptr);
        updateWindowTitle();
        return;
    }

    m_binder->bind(newTab);
    m_palette->updateICList(icListFile());

    // Apply the minimap visibility preference to the newly activated tab. Position/size are
    // per-tab (each WorkSpace restores its own from Settings::minimapGeometry() on first
    // layout), so there's nothing to push here for those.
    newTab->setMinimapVisible(Settings::minimapVisible());

    // Hide management buttons for inline IC tabs (they use currentFile/currentDir which are empty)
    setICButtonsVisible(!newTab->isInlineIC());
    refreshICButtonsEnabled();

    // Re-attach exercise overlay to the arriving tab
    if (m_exerciseEngine && m_exerciseEngine->isActive() && m_exerciseOverlay) {
        m_exerciseEngine->setScene(newTab->scene());
        m_exerciseOverlay->setParent(newTab);
        newTab->setExerciseOverlay(m_exerciseOverlay);
        m_exerciseOverlay->repositionToParent();
        m_exerciseOverlay->show();
        m_exerciseOverlay->raise();
    }

    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    auto *tab = currentTab();
    if (!tab) {
        setWindowTitle(QStringLiteral("wiRedPanda " APP_VERSION));
        setWindowModified(false);
        return;
    }

    // "<name>[*] — wiRedPanda <version>": Qt swaps "[*]" for "*" when the window is marked
    // modified, and drops it otherwise (a native dot on macOS).
    setWindowTitle(i18n("%1[*] — wiRedPanda %2")
                       .arg(m_workspaceManager->currentTabName(), QStringLiteral(APP_VERSION)));
    setWindowModified(!tab->scene()->undoStack()->isClean());
}

void MainWindow::on_actionGates_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Gates: %1").arg(checked));
        if (!currentTab()) {
            return;
        }

        // Wire visibility depends on gates being visible: if gates are hidden, wires
        // make no sense and should be hidden too.  Re-enable the wire toggle only when
        // gates are shown so the user can't end up with floating wires.
        m_ui->actionWires->setEnabled(checked);
        currentTab()->scene()->showWires(checked ? m_ui->actionWires->isChecked() : checked);
        currentTab()->scene()->showGates(checked);
    });
}

void MainWindow::exportToArduino(QString fileName)
{
    m_exportController->exportToArduino(std::move(fileName));
}

void MainWindow::exportToSystemVerilog(QString fileName)
{
    m_exportController->exportToSystemVerilog(std::move(fileName));
}

void MainWindow::exportToWaveFormFile(const QString &fileName)
{
    m_exportController->exportToWaveFormFile(fileName);
}

void MainWindow::exportToWaveFormTerminal()
{
    m_exportController->exportToWaveFormTerminal();
}

void MainWindow::on_actionZoomIn_triggered() const
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->view()->zoomIn();
    });
}

void MainWindow::on_actionZoomOut_triggered() const
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->view()->zoomOut();
    });
}

void MainWindow::on_actionResetZoom_triggered() const
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->view()->resetZoom();
    });
}

void MainWindow::on_actionZoomToFit_triggered() const
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->view()->zoomToFit();
    });
}

void MainWindow::updateRecentFileActions()
{
    const auto files = m_recentFiles->recentFiles();
    const int numRecentFiles = static_cast<int>(qMin(files.size(), RecentFiles::maxFiles));

    if (numRecentFiles > 0) {
        m_ui->menuRecentFiles->setEnabled(true);
    }

    auto actions = m_ui->menuRecentFiles->actions();

    // The menu has exactly RecentFiles::maxFiles pre-allocated actions; update
    // visible ones in order, hide the rest.  Prefix "&1", "&2" … adds a mnemonic
    // so the entries are keyboard-accessible without a mouse.
    for (int i = 0; i < numRecentFiles; ++i) {
        const QString text = "&" + QString::number(i + 1) + " " + QFileInfo(files.at(i)).fileName();
        actions.at(i)->setText(text);
        actions.at(i)->setData(files.at(i));
        actions.at(i)->setVisible(true);
    }

    for (int i = numRecentFiles; i < RecentFiles::maxFiles; ++i) {
        actions.at(i)->setVisible(false);
    }
}

void MainWindow::openRecentFile()
{
    if (auto *action = qobject_cast<QAction *>(sender())) {
        sentryBreadcrumb("file", QStringLiteral("Open recent file"));
        loadPandaFile(action->data().toString());
    }
}

void MainWindow::createRecentFileActions()
{
    m_ui->menuRecentFiles->clear();

    for (int i = 0; i < RecentFiles::maxFiles; ++i) {
        auto *action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
        m_ui->menuRecentFiles->addAction(action);
    }

    updateRecentFileActions();
}

void MainWindow::retranslateUi()
{
    m_ui->retranslateUi();
    m_ui->elementEditor->retranslateUi();
    m_palette->retranslateLabels();

    for (int index = 0; index < m_ui->tab->count(); ++index) {
        auto *workspace = qobject_cast<WorkSpace *>(m_ui->tab->widget(index));
        if (!workspace) {
            continue;
        }
        auto *scene = workspace->scene();
        if (!scene) {
            continue;
        }
        auto *undoStack = scene->undoStack();
        if (!undoStack) {
            continue;
        }
        QString text;
        if (workspace->isInlineIC()) {
            text = "[" + workspace->inlineBlobName() + "]";
        } else {
            auto fileInfo = workspace->fileInfo();
            text = fileInfo.exists() ? fileInfo.fileName() : i18n("New Project");
        }

        if (!undoStack->isClean()) {
            text += "*";
        }

        m_ui->tab->setTabText(index, text);

        scene->retranslateUi();

        for (auto *elm : workspace->scene()->elements()) {
            elm->retranslate();
        }
    }

    if (m_exerciseEngine->isActive()) {
        m_exerciseEngine->retranslate();
    }
    if (m_tourEngine->isActive()) {
        m_tourEngine->retranslate();
    }
}

void MainWindow::loadTranslation(const QString &language)
{
    m_languageManager->loadTranslation(language);
}

void MainWindow::populateLanguageMenu()
{
    m_ui->menuLanguage->clear();

    auto *languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);

    for (const QString &langCode : m_languageManager->availableLanguages()) {
        auto *action = new QAction(m_languageManager->displayName(langCode), this);
        action->setCheckable(true);
        action->setData(langCode);
        action->setIcon(QIcon(m_languageManager->flagIcon(langCode)));

        if (langCode == Settings::language() || (langCode == "en" && Settings::language().isEmpty())) {
            action->setChecked(true);
        }

        languageGroup->addAction(action);
        m_ui->menuLanguage->addAction(action);

        connect(action, &QAction::triggered, this, [this, langCode]() {
            m_languageManager->loadTranslation(langCode);
        });
    }
}

void MainWindow::on_actionPlay_toggled(const bool checked)
{
    sentryBreadcrumb("simulation", QStringLiteral("Play toggled: %1").arg(checked));
    if (!currentTab()) {
        return;
    }

    auto *simulation = currentTab()->simulation();

    // The action is checkable; its toggled(bool) signal drives start/stop directly.
    checked ? simulation->start() : simulation->stop();
}

void MainWindow::on_actionRestart_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("simulation", QStringLiteral("Simulation restart"));
        if (!currentTab()) {
            return;
        }

        currentTab()->simulation()->restart();
    });
}

void MainWindow::populateMenu(QSpacerItem *spacer, const QStringList &names, QLayout *layout)
{
    // The spacer must be removed before inserting widgets so it stays at the
    // bottom after they are added; it is put back at the end of this function.
    layout->removeItem(spacer);

    // Each element type gets two labels: one in its own category panel and a
    // duplicate in the shared search panel so search can find everything in one place.
    for (const auto &name : names) {
        auto type = ElementFactory::textToType(name);
        auto pixmap(ElementFactory::pixmap(type));
        layout->addWidget(new ElementLabel(pixmap, type, name, this));
        m_ui->scrollAreaWidgetContents_Search->layout()->addWidget(new ElementLabel(pixmap, type, name, this));
    }

    layout->addItem(spacer);
}

void MainWindow::on_actionFastMode_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Fast mode: %1").arg(checked));
        setFastMode(checked);
        Settings::setFastMode(checked);
    });
}

void MainWindow::on_actionWaveform_triggered()
{
    Application::guardedSlot(this, [this] {
        if (m_bwd) {
            m_bwd->raise();
            m_bwd->activateWindow();
            return;
        }
        if (!currentTab()) {
            return;
        }

        sentryBreadcrumb("ui", QStringLiteral("Waveform dialog opened"));
        qCDebug(zero) << "BD fileName: " << currentTab()->dolphinFileName();
        auto *bwd = new BewavedDolphin(currentTab()->scene(), true, this, this);
        bwd->createWaveform(currentTab()->dolphinFileName());
        m_bwd = bwd;
        connect(bwd, &QObject::destroyed, this, [this] {
            if (m_exerciseOverlay && m_exerciseEngine && m_exerciseEngine->isActive()) {
                m_exerciseOverlay->hide();
                m_exerciseOverlay->setParent(nullptr);
            }
        });
        bwd->show();
    });
}

void MainWindow::on_actionLightTheme_triggered()
{
    Application::guardedSlot(qApp, [] {
        sentryBreadcrumb("ui", QStringLiteral("Theme: light"));
        ThemeManager::setTheme(Theme::Light);
    });
}

void MainWindow::on_actionDarkTheme_triggered()
{
    Application::guardedSlot(qApp, [] {
        sentryBreadcrumb("ui", QStringLiteral("Theme: dark"));
        ThemeManager::setTheme(Theme::Dark);
    });
}

void MainWindow::on_actionSystemTheme_triggered()
{
    Application::guardedSlot(qApp, [] {
        sentryBreadcrumb("ui", QStringLiteral("Theme: system"));
        ThemeManager::setTheme(Theme::System);
    });
}

void MainWindow::updateTheme()
{
    switch (ThemeManager::theme()) {
    case Theme::Dark:   m_ui->actionDarkTheme->setChecked(true);   break;
    case Theme::Light:  m_ui->actionLightTheme->setChecked(true);  break;
    case Theme::System: m_ui->actionSystemTheme->setChecked(true); break;
    }

    m_palette->updateTheme();
    m_ui->elementEditor->updateTheme();
}

void MainWindow::on_actionFlipHorizontally_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->flipHorizontally();
    });
}

void MainWindow::on_actionFlipVertically_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->flipVertically();
    });
}

void MainWindow::on_actionAlignLeft_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->alignLeft();
    });
}

void MainWindow::on_actionAlignRight_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->alignRight();
    });
}

void MainWindow::on_actionAlignTop_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->alignTop();
    });
}

void MainWindow::on_actionAlignBottom_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->alignBottom();
    });
}

void MainWindow::on_actionAlignHorizontalCenter_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->alignHorizontalCenter();
    });
}

void MainWindow::on_actionAlignVerticalCenter_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->alignVerticalCenter();
    });
}

void MainWindow::on_actionDistributeHorizontally_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->distributeHorizontally();
    });
}

void MainWindow::on_actionDistributeVertically_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab()) {
            return;
        }

        currentTab()->scene()->distributeVertically();
    });
}

QString MainWindow::dolphinFileName()
{
    return m_workspaceManager->dolphinFileName();
}

void MainWindow::setDolphinFileName(const QString &fileName)
{
    m_workspaceManager->setDolphinFileName(fileName);
}

void MainWindow::on_actionFullscreen_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ui", QStringLiteral("Fullscreen toggled"));
        isFullScreen() ? showNormal() : showFullScreen();
    });
}

void MainWindow::on_actionMute_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Mute toggled"));
        if (!currentTab()) {
            return;
        }

        currentTab()->simulation()->setUserMuted(checked);
        m_ui->actionMute->setText(checked ? i18n("Unmute") : i18n("Mute"));
    });
}

void MainWindow::on_actionLabelsUnderIcons_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Labels under icons: %1").arg(checked));
        m_ui->mainToolBar->setToolButtonStyle(checked ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
        Settings::setLabelsUnderIcons(checked);
    });
}

void MainWindow::on_actionICPreview_triggered(const bool checked)
{
    Application::guardedSlot(this, [checked] {
        sentryBreadcrumb("ui", QStringLiteral("IC preview: %1").arg(checked));
        Settings::setIcPreviewDisabled(!checked);
    });
}

void MainWindow::on_actionCheckForUpdates_triggered(const bool checked)
{
    Application::guardedSlot(this, [checked] {
        sentryBreadcrumb("ui", QStringLiteral("Auto update checks: %1").arg(checked));
        Settings::setUpdateChecksDisabled(!checked);
    });
}

void MainWindow::on_actionShowMinimap_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Show minimap: %1").arg(checked));
        Settings::setMinimapVisible(checked);
        if (currentTab()) currentTab()->setMinimapVisible(checked);
    });
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowActivate: {
        // Resume simulation when the window regains focus.
        if (m_ui->actionPlay->isChecked()) {
            on_actionPlay_toggled(true);
        }
        break;
    }

    case QEvent::WindowDeactivate: {
        // Pause simulation when the window loses focus unless the user opted in
        // to background simulation (useful for demoing circuits on a second display).
        if (!m_ui->actionBackground_Simulation->isChecked()) {
            on_actionPlay_toggled(false);
        }
        break;
    }

    default: break;
    };

    return QMainWindow::event(event);
}

void MainWindow::startExercise(const QString &resourcePath)
{
    if (!currentTab() || resourcePath.isEmpty()) {
        return;
    }
    if (!m_exerciseEngine->loadFromResource(resourcePath)) {
        qWarning() << "ExerciseEngine: failed to load" << resourcePath;
        return;
    }

    if (currentTab()) {
        currentTab()->setExerciseOverlay(nullptr);
    }
    delete m_exerciseOverlay;
    m_exerciseOverlay = new ExerciseOverlay(m_exerciseEngine, currentTab());

    // m_exerciseEngine outlives any single exercise run, so a prior startExercise() call's
    // click-handling connection (below) must be dropped before reconnecting, or replaying an
    // exercise fires accumulated handlers once per past run.
    disconnect(m_exerciseEngine, &ExerciseEngine::stepChanged, this, nullptr);

    // Must precede stepChanged→onStepChanged so actions run before the overlay updates.
    connect(m_exerciseEngine, &ExerciseEngine::stepChanged, this,
            [this](int, int, const ExerciseStep &step) {
        for (const QString &id : step.click) clickTarget(id);

        if (!m_exerciseOverlay) {
            return;
        }
        const bool wantBwd = (step.context == "bwd");
        if (wantBwd && m_bwd) {
            m_exerciseOverlay->setParent(m_bwd->centralWidget());
            m_bwd->setExerciseOverlay(m_exerciseOverlay);
            if (currentTab()) {
                currentTab()->setExerciseOverlay(nullptr);
            }
        } else if (!wantBwd && currentTab()) {
            m_exerciseOverlay->setParent(currentTab());
            currentTab()->setExerciseOverlay(m_exerciseOverlay);
            if (m_bwd) {
                m_bwd->setExerciseOverlay(nullptr);
            }
        }
        m_exerciseOverlay->repositionToParent();
        m_exerciseOverlay->show();
        m_exerciseOverlay->raise();
    });
    connect(m_exerciseEngine, &ExerciseEngine::stepChanged,
            m_exerciseOverlay, &ExerciseOverlay::onStepChanged);
    connect(m_exerciseEngine, &ExerciseEngine::exerciseCompleted,
            m_exerciseOverlay, &ExerciseOverlay::onExerciseCompleted);
    connect(m_exerciseEngine, &ExerciseEngine::retranslated,
            m_exerciseOverlay, &ExerciseOverlay::onRetranslated);
    connect(m_exerciseOverlay, &ExerciseOverlay::closeRequested, this, [this] {
        if (!m_exerciseOverlay) return;
        ExerciseOverlay *overlay = m_exerciseOverlay;
        m_exerciseOverlay = nullptr;
        m_exerciseEngine->stop();
        if (currentTab()) {
            currentTab()->setExerciseOverlay(nullptr);
        }
        if (m_bwd) {
            m_bwd->setExerciseOverlay(nullptr);
        }
        overlay->deleteLater();
    });

    auto *esc = new QShortcut(QKeySequence(Qt::Key_Escape), m_exerciseOverlay);
    connect(esc, &QShortcut::activated, m_exerciseOverlay, &ExerciseOverlay::closeRequested);

    currentTab()->setExerciseOverlay(m_exerciseOverlay);
    m_exerciseEngine->setScene(currentTab()->scene());
    m_exerciseEngine->start();

    m_exerciseOverlay->repositionToParent();
    m_exerciseOverlay->show();
    m_exerciseOverlay->raise();
}

void MainWindow::startTour(const QString &resourcePath)
{
    if (resourcePath.isEmpty()) {
        return;
    }
    if (!m_tourEngine->loadFromResource(resourcePath)) {
        qWarning() << "TourEngine: failed to load" << resourcePath;
        return;
    }

    delete m_tourOverlay;
    m_tourOverlay = new TourOverlay(m_tourEngine, this);
    m_tourOverlay->setTargetResolver([this](const QString &id) {
        return resolveTourTarget(id);
    });

    // m_tourEngine outlives any single tour run, so a prior startTour() call's click-handling
    // connection (below) must be dropped before reconnecting, or replaying a tour fires
    // accumulated handlers once per past run.
    disconnect(m_tourEngine, &TourEngine::stepChanged, this, nullptr);

    // Must be connected before stepChanged→onStepChanged so click executes before
    // resolveTourTarget computes widget rects.
    connect(m_tourEngine, &TourEngine::stepChanged, this,
            [this](int, int, const TourStep &step) {
        for (const QString &id : step.click) clickTarget(id);

        TourOverlay *overlay = m_tourOverlay;
        if (!overlay) {
            return;
        }
        const bool wantBwd = step.target.startsWith("bwd:");
        if (wantBwd && m_bwd) {
            if (overlay->parentWidget() != m_bwd) {
                overlay->setParentWindow(m_bwd);
            }
            overlay->show();
            overlay->raise();
        } else if (!wantBwd && overlay->parentWidget() != this) {
            overlay->setParentWindow(this);
            overlay->show();
            overlay->raise();
        }
    });
    connect(m_tourEngine, &TourEngine::stepChanged,
            m_tourOverlay, &TourOverlay::onStepChanged);
    connect(m_tourEngine, &TourEngine::tourCompleted,
            m_tourOverlay, &TourOverlay::onTourFinished);
    connect(m_tourEngine, &TourEngine::tourStopped,
            m_tourOverlay, &TourOverlay::onTourFinished);
    connect(m_tourEngine, &TourEngine::retranslated,
            m_tourOverlay, &TourOverlay::onRetranslated);
    connect(m_tourOverlay, &TourOverlay::closeRequested, this, [this] {
        if (!m_tourOverlay) return;
        TourOverlay *overlay = m_tourOverlay;
        m_tourOverlay = nullptr;
        m_tourEngine->stop();
        overlay->deleteLater();
    });

    auto *esc = new QShortcut(QKeySequence(Qt::Key_Escape), m_tourOverlay);
    connect(esc, &QShortcut::activated, m_tourOverlay, &TourOverlay::closeRequested);

    m_tourEngine->start();

    m_tourOverlay->show();
    m_tourOverlay->raise();
}

QRect MainWindow::resolveTourTarget(const QString &id) const
{
    if (!m_tourOverlay || id.isEmpty() || id == "none") {
        return {};
    }

    auto mapWidget = [this](QWidget *w) -> QRect {
        if (!w) return {};
        const QPoint topLeft = m_tourOverlay->mapFromGlobal(w->mapToGlobal(QPoint(0, 0)));
        return QRect(topLeft, w->size());
    };

    if (id == "toolbar")        return mapWidget(m_ui->mainToolBar->widgetForAction(m_ui->actionWaveform));
    if (id == "elementPalette") return mapWidget(m_ui->tabElements);
    if (id == "gatesTab") {
        QTabBar *bar = m_ui->tabElements->tabBar();
        const QRect tabRect = bar->tabRect(1);
        const QPoint topLeft = m_tourOverlay->mapFromGlobal(bar->mapToGlobal(tabRect.topLeft()));
        return QRect(topLeft, tabRect.size());
    }
    if (id == "ioTab") {
        QTabBar *bar = m_ui->tabElements->tabBar();
        const QRect tabRect = bar->tabRect(0);
        const QPoint topLeft = m_tourOverlay->mapFromGlobal(bar->mapToGlobal(tabRect.topLeft()));
        return QRect(topLeft, tabRect.size());
    }
    if (id == "canvasArea" && currentTab()) return mapWidget(currentTab()->view());
    if (id == "elementEditor")  return mapWidget(m_ui->elementEditor);
    if (id == "searchBar")      return mapWidget(m_ui->lineEditSearch);

    BewavedDolphin *bwd = m_bwd;
    if (id.startsWith("bwd:") && bwd) {
        auto mapBwd = [this](QWidget *w) -> QRect {
            if (!w || !m_tourOverlay) {
                return {};
            }
            const QPoint tl = m_tourOverlay->mapFromGlobal(w->mapToGlobal(QPoint(0, 0)));
            return QRect(tl, w->size());
        };
        const QString sub = id.sliced(4);
        if (sub == "tableView") return mapBwd(bwd->signalTableView());
        if (sub == "toolbar")   return mapBwd(bwd->mainToolBar()->widgetForAction(bwd->actionCombinational()));
        if (sub == "menuBar")   return mapBwd(bwd->menuBar());
        return {};
    }

    return {};
}

void MainWindow::clickTarget(const QString &id)
{
    if      (id == "ioTab")          m_ui->tabElements->setCurrentIndex(0);
    else if (id == "gatesTab")       m_ui->tabElements->setCurrentIndex(1);
    else if (id == "combinational")  m_ui->tabElements->setCurrentIndex(2);
    else if (id == "memoryTab")      m_ui->tabElements->setCurrentIndex(3);
    else if (id == "actionPlay")     m_ui->actionPlay->trigger();
    else if (id == "actionWaveform") m_ui->actionWaveform->trigger();
    else if (id == "bwd:actionCombinational" && m_bwd) m_bwd->triggerCombinational();
    else if (id == "setupElementEditorDemo") {
        if (!currentTab()) {
            return;
        }
        Scene *scene = currentTab()->scene();
        scene->clearSelection();
        auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
        sw->setPos(0, 0);
        // AddItemsCommand registers the element with the scene's Simulation
        // (setCircuitUpdateRequired() in redo()) and selects it for us.
        scene->receiveCommand(new AddItemsCommand({sw}, scene));
    }
    else if (id == "setupWaveformDemo") {
        if (!currentTab()) {
            return;
        }
        Scene *scene = currentTab()->scene();
        scene->clearSelection();
        auto *clock1 = ElementFactory::buildElement(ElementType::Clock);
        auto *clock2 = ElementFactory::buildElement(ElementType::Clock);
        auto *gate   = ElementFactory::buildElement(ElementType::And);
        auto *led    = ElementFactory::buildElement(ElementType::Led);
        clock1->setPos(-160, -60);
        clock2->setPos(-160,  60);
        gate->setPos(0, 0);
        led->setPos(160, 0);

        auto *conn1 = new Connection();
        conn1->setStartPort(clock1->outputPort(0));
        conn1->setEndPort(gate->inputPort(0));
        auto *conn2 = new Connection();
        conn2->setStartPort(clock2->outputPort(0));
        conn2->setEndPort(gate->inputPort(1));
        auto *conn3 = new Connection();
        conn3->setStartPort(gate->outputPort(0));
        conn3->setEndPort(led->inputPort(0));

        // AddItemsCommand auto-discovers conn1..conn3 via port traversal (loadList())
        // and registers all 7 items with the scene's Simulation, fixing the
        // frozen-clock bug that raw scene->addItem() caused.
        scene->receiveCommand(new AddItemsCommand({clock1, clock2, gate, led}, scene));

        conn1->updatePath();
        conn2->updatePath();
        conn3->updatePath();
    }
}
