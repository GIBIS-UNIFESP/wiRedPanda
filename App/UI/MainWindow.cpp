// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/MainWindow.h"

#ifdef Q_OS_WASM
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include <QActionGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QLocale>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmapCache>
#include <QProgressDialog>
#include <QPushButton>
#include <QSaveFile>
#include <QShortcut>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QVBoxLayout>

#ifdef Q_OS_MAC
#include <QSvgRenderer>
#endif

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/Core/UpdateChecker.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"
#ifndef USE_KDE_FRAMEWORKS
#include "App/IO/RecentFiles.h"
#endif
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/CircuitExporter.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/LanguageManager.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/MessageDialog.h"
#include "App/Versions.h"

#ifdef USE_KDE_FRAMEWORKS
#include <KActionCollection>
#include <KConfigGroup>
#include <KNSWidgets/Dialog>
#include <KNotification>
#include <KRecentFilesAction>
#include <KSharedConfig>
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

    qCDebug(zero) << "Opening file if not empty.";
    if (!fileName.isEmpty()) {
        loadPandaFile(fileName);
    }

    // 100 000 KB cache limit — large circuits with many IC pixmaps benefit from generous caching.
    QPixmapCache::setCacheLimit(100000);

    qCDebug(zero) << "Adding examples to menu";
    setupExamplesMenu();
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
                             const QKeySequence &shortcut = {},
                             const QString &themeName = {}) -> QAction * {
        auto *a = new QAction(this);
        a->setText(text);
        // Phase 6c: prefer the freedesktop/Breeze theme icon, fall back to bundled SVG.
        if (!themeName.isEmpty()) {
            a->setIcon(QIcon::fromTheme(themeName, QIcon(iconPath)));
        } else if (!iconPath.isEmpty()) {
            a->setIcon(QIcon(iconPath));
        }
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

    // Recent files action — must exist in the collection before setupGUI() so
    // wiredpandaui.rc's <Action name="file_open_recent"/> resolves. Persisted
    // entries are loaded and the addRecentFile signal wired in setupRecentFiles().
    m_recentFilesAction = KStandardAction::openRecent(
        this,
        [this](const QUrl &url) {
            sentryBreadcrumb("file", QStringLiteral("Open recent file"));
            loadPandaFile(url.toLocalFile());
        },
        actionCollection());

    // Phase 8: KNewStuff "Download Circuits" entry. The action lives only on the
    // KDE path; non-KDE builds skip it entirely.
    auto *downloadAction = addAction(u"wiredpanda_download_circuits"_s, u"actionDownloadCircuits"_s,
        i18n("&Download Circuits…"), {}, {}, u"get-hot-new-stuff"_s);
    connect(downloadAction, &QAction::triggered, this, &MainWindow::downloadCircuits);

    // ── Custom actions (created as window children, registered in collection) ───

    m_ui->actionReloadFile = addAction(u"wiredpanda_reload_file"_s, u"actionReloadFile"_s, i18n("Re&load File"),
        u":/Interface/Toolbar/reloadFile.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_F5), u"view-refresh"_s);
    m_ui->actionDelete = addAction(u"wiredpanda_delete"_s, u"actionDelete"_s, i18n("&Delete"),
        u":/Interface/Toolbar/delete.svg"_s, QKeySequence::Delete, u"edit-delete"_s);
    m_ui->actionRename = addAction(u"wiredpanda_rename"_s, u"actionRename"_s, i18n("&Rename"),
        u":/Interface/Toolbar/rename.svg"_s, QKeySequence(Qt::Key_F2), u"edit-rename"_s);
    m_ui->actionChangeTrigger = addAction(u"wiredpanda_change_trigger"_s, u"actionChangeTrigger"_s, i18n("Cha&nge Trigger"),
        u":/Components/Input/buttonOff.svg"_s, QKeySequence(Qt::Key_F3));
    m_ui->actionRotateRight = addAction(u"wiredpanda_rotate_right"_s, u"actionRotateRight"_s, i18n("R&otate right"),
        u":/Interface/Toolbar/rotateR.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_R), u"object-rotate-right"_s);
    m_ui->actionRotateLeft = addAction(u"wiredpanda_rotate_left"_s, u"actionRotateLeft"_s, i18n("Rotate &left"),
        u":/Interface/Toolbar/rotateL.svg"_s, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R), u"object-rotate-left"_s);
    m_ui->actionFlipHorizontally = addAction(u"wiredpanda_flip_h"_s, u"actionFlipHorizontally"_s, i18n("&Flip horizontally"),
        {}, QKeySequence(Qt::CTRL | Qt::Key_H), u"object-flip-horizontal"_s);
    m_ui->actionFlipVertically = addAction(u"wiredpanda_flip_v"_s, u"actionFlipVertically"_s, i18n("Flip &vertically"),
        {}, {}, u"object-flip-vertical"_s);
    m_ui->actionClearSelection = addAction(u"wiredpanda_clear_selection"_s, u"actionClearSelection"_s, i18n("Cl&ear selection"),
        u":/Interface/Toolbar/clearSelection.svg"_s, QKeySequence(Qt::Key_Escape), u"edit-clear"_s);
    m_ui->actionResetZoom = addAction(u"wiredpanda_reset_zoom"_s, u"actionResetZoom"_s, i18n("&Reset Zoom"),
        u":/Interface/Toolbar/zoomReset.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_0), u"zoom-original"_s);
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
        // Two-state icon (off=play, on=pause). Theme icons can't be combined in
        // one QIcon under different states, so keep the bundled SVGs here.
        QIcon playIcon(u":/Interface/Toolbar/play.svg"_s);
        playIcon.addFile(u":/Interface/Toolbar/pause.svg"_s, QSize(), QIcon::Normal, QIcon::On);
        m_ui->actionPlay->setIcon(playIcon);
    }
    m_ui->actionRestart = addAction(u"wiredpanda_restart"_s, u"actionRestart"_s, i18n("&Restart"),
        u":/Interface/Toolbar/reset.svg"_s, {}, u"media-playback-stop"_s);
    m_ui->actionWaveform = addAction(u"wiredpanda_waveform"_s, u"actionWaveform"_s, i18n("&Waveform"),
        u":/Interface/Toolbar/dolphin_icon.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_W));
    m_ui->actionMute = addAction(u"wiredpanda_mute"_s, u"actionMute"_s, i18n("Mute"),
        u":/Components/Output/Buzzer/BuzzerOff.svg"_s, QKeySequence(Qt::CTRL | Qt::Key_M));
    m_ui->actionMute->setCheckable(true);
    {
        // Two-state icon (off=buzzer, on=muted). Theme icons can't be combined in
        // one QIcon under different states, so keep the bundled SVGs here.
        QIcon muteIcon(u":/Components/Output/Buzzer/BuzzerOff.svg"_s);
        muteIcon.addFile(u":/Interface/Toolbar/mute.svg"_s, QSize(), QIcon::Normal, QIcon::On);
        m_ui->actionMute->setIcon(muteIcon);
    }
    m_ui->actionBackground_Simulation = addAction(u"wiredpanda_background_simulation"_s,
        u"actionBackground_Simulation"_s, i18n("Background Simulation"), u":/Interface/Toolbar/reset.svg"_s);
    m_ui->actionBackground_Simulation->setCheckable(true);
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
        i18n("&About"), u":/Interface/Toolbar/help.svg"_s, QKeySequence(Qt::Key_F1), u"help-about"_s);
    m_ui->actionAboutQt = addAction(u"wiredpanda_about_qt"_s, u"actionAboutQt"_s,
        i18n("About &Qt"), u":/Interface/Toolbar/helpQt.svg"_s);
    m_ui->actionAboutThisVersion = addAction(u"wiredpanda_about_this_version"_s, u"actionAboutThisVersion"_s,
        i18n("About this version"), {});
    m_ui->actionShortcutsAndTips = addAction(u"wiredpanda_shortcuts_tips"_s, u"actionShortcutsAndTips"_s,
        i18n("Shortcuts and Tips"), {});
    m_ui->actionReportTranslationError = addAction(u"wiredpanda_report_translation"_s, u"actionReportTranslationError"_s,
        i18n("Report Translation Error"), u":/Interface/Toolbar/help.svg"_s);

    // ── Dynamic menus (populated at runtime) ────────────────────────────────
    // Set objectName on each so tests using findChild<QMenu*>("menuXxx") work.
    // menuRecentFiles is owned by KRecentFilesAction (built lazily); we point
    // m_ui->menuRecentFiles at it after setupGUI() runs, below.
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

    // Recent-files submenu is inserted automatically by setupGUI() because
    // wiredpandaui.rc references the "file_open_recent" action; just add a
    // separator after it and expose the menu pointer for findChild() tests.
    if (m_ui->menuFile) {
        QAction *saveAction = actionCollection()->action(u"file_save"_s);
        m_ui->menuFile->insertSeparator(saveAction);
    }
    // KRecentFilesAction owns its auto-built menu (a QMenu with no QObject
    // parent), so findChild() on the window can't reach it. Expose it on
    // m_ui->menuRecentFiles for code that already holds the pointer; tests
    // should fetch it via actionCollection()->action("file_open_recent").
    if (auto *recentMenu = m_recentFilesAction->menu()) {
        recentMenu->setObjectName(u"menuRecentFiles"_s);
        m_ui->menuRecentFiles = recentMenu;
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
}

void MainWindow::setupRecentFiles()
{
#ifdef USE_KDE_FRAMEWORKS
    // m_recentFilesAction was created in setupKdeActions() before setupGUI()
    // so the rc file's "file_open_recent" reference resolved. Here we just
    // load persisted entries and wire the addRecentFile signal.
    m_recentFilesAction->loadEntries(KSharedConfig::openConfig()->group(u"RecentFiles"_s));
    connect(this, &MainWindow::addRecentFile, this, [this](const QString &path) {
        // KRecentFilesAction silently drops URLs under QDir::tempPath().
        m_recentFilesAction->addUrl(QUrl::fromLocalFile(path), QFileInfo(path).fileName());
        m_recentFilesAction->saveEntries(KSharedConfig::openConfig()->group(u"RecentFiles"_s));
    });
#else
    m_recentFiles = new RecentFiles(this);
    connect(this,          &MainWindow::addRecentFile,         m_recentFiles, &RecentFiles::addRecentFile);
    connect(m_recentFiles, &RecentFiles::recentFilesUpdated,   this,          &MainWindow::updateRecentFileActions);
    createRecentFileActions();
#endif
}

void MainWindow::setupExamplesMenu()
{
    const QString appDir = QCoreApplication::applicationDirPath();

    const QStringList searchPaths = {
        appDir + "/Examples",                       // Windows / dev builds
#ifdef Q_OS_MACOS
        appDir + "/../Resources/Examples",          // macOS app bundle
#endif
#ifdef Q_OS_LINUX
        qEnvironmentVariable("APPDIR") + "/usr/share/wiredpanda/Examples",  // AppImage
#endif
#ifdef Q_OS_WASM
        QStringLiteral("/Examples"),                // WASM virtual filesystem (--preload-file)
#endif
        QStringLiteral("Examples"),                 // CWD fallback (development)
    };

    QString examplesPath;
    for (const auto &path : searchPaths) {
        if (!path.isEmpty() && QDir(path).exists()) {
            examplesPath = path;
            break;
        }
    }

    if (!examplesPath.isEmpty()) {
        const auto entryList = QDir(examplesPath).entryList({"*.panda"}, QDir::Files);

        for (const auto &entry : entryList) {
            auto *action = new QAction(entry, this);

            connect(action, &QAction::triggered, this, [this, examplesPath] {
                if (auto *senderAction = qobject_cast<QAction *>(sender())) {
                    loadPandaFile(examplesPath + "/" + senderAction->text());
                }
            });

            m_ui->menuExamples->addAction(action);
        }
    }

    if (m_ui->menuExamples->isEmpty()) {
        m_ui->menuExamples->menuAction()->setVisible(false);
    }
}

void MainWindow::setupShortcuts()
{
    // [ / ] cycle a selected element's primary property (e.g. input size).
    // { / } cycle a secondary property; < / > morph through element variants.
    // Scene connections are made in connectTab() so they follow the active tab.
    auto *searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    m_prevMainPropShortcut  = new QShortcut(QKeySequence("["), this);
    m_nextMainPropShortcut  = new QShortcut(QKeySequence("]"), this);
    m_prevSecndPropShortcut = new QShortcut(QKeySequence("{"), this);
    m_nextSecndPropShortcut = new QShortcut(QKeySequence("}"), this);
    m_changePrevElmShortcut = new QShortcut(QKeySequence("<"), this);
    m_changeNextElmShortcut = new QShortcut(QKeySequence(">"), this);

    connect(searchShortcut, &QShortcut::activated, m_ui->lineEditSearch, qOverload<>(&QWidget::setFocus));
}

void MainWindow::setupConnections()
{
    connect(m_ui->tab, &QTabWidget::currentChanged,    this, &MainWindow::tabChanged);
    connect(m_ui->tab, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    connect(m_ui->actionAbout,                 &QAction::triggered,       this,                &MainWindow::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,               &QAction::triggered,       this,                &MainWindow::on_actionAboutQt_triggered);
    connect(m_ui->actionAboutThisVersion,      &QAction::triggered,       this,                &MainWindow::aboutThisVersion);
    connect(m_ui->actionReportTranslationError,&QAction::triggered,       this,                &MainWindow::on_actionReportTranslationError_triggered);
    connect(m_ui->actionChangeTrigger,         &QAction::triggered,       m_ui->elementEditor, &ElementEditor::changeTriggerAction);
    connect(m_ui->actionDarkTheme,             &QAction::triggered,       this,                &MainWindow::on_actionDarkTheme_triggered);
    connect(m_ui->actionSystemTheme,           &QAction::triggered,       this,                &MainWindow::on_actionSystemTheme_triggered);
    connect(m_ui->actionExit,                  &QAction::triggered,       this,                &MainWindow::on_actionExit_triggered);
    connect(m_ui->actionExportToArduino,       &QAction::triggered,       this,                &MainWindow::on_actionExportToArduino_triggered);
    connect(m_ui->actionExportToSystemVerilog, &QAction::triggered,       this,                &MainWindow::on_actionExportToSystemVerilog_triggered);
    connect(m_ui->actionExportToImage,         &QAction::triggered,       this,                &MainWindow::on_actionExportToImage_triggered);
    connect(m_ui->actionExportToPdf,           &QAction::triggered,       this,                &MainWindow::on_actionExportToPdf_triggered);
    connect(m_ui->actionFastMode,              &QAction::triggered,       this,                &MainWindow::on_actionFastMode_triggered);
    connect(m_ui->actionFlipHorizontally,      &QAction::triggered,       this,                &MainWindow::on_actionFlipHorizontally_triggered);
    connect(m_ui->actionFlipVertically,        &QAction::triggered,       this,                &MainWindow::on_actionFlipVertically_triggered);
    connect(m_ui->actionFullscreen,            &QAction::triggered,       this,                &MainWindow::on_actionFullscreen_triggered);
    connect(m_ui->actionGates,                 &QAction::triggered,       this,                &MainWindow::on_actionGates_triggered);
    connect(m_ui->actionLabelsUnderIcons,      &QAction::triggered,       this,                &MainWindow::on_actionLabelsUnderIcons_triggered);
    connect(m_ui->actionLightTheme,            &QAction::triggered,       this,                &MainWindow::on_actionLightTheme_triggered);
    connect(m_ui->actionMute,                  &QAction::triggered,       this,                &MainWindow::on_actionMute_triggered);
    connect(m_ui->actionNew,                   &QAction::triggered,       this,                &MainWindow::on_actionNew_triggered);
    connect(m_ui->actionOpen,                  &QAction::triggered,       this,                &MainWindow::on_actionOpen_triggered);
    connect(m_ui->actionPlay,                  &QAction::toggled,         this,                &MainWindow::on_actionPlay_toggled);
    connect(m_ui->actionReloadFile,            &QAction::triggered,       this,                &MainWindow::on_actionReloadFile_triggered);
    connect(m_ui->actionRename,                &QAction::triggered,       m_ui->elementEditor, &ElementEditor::renameAction);

    // ElementEditor IC sub-circuit actions
    connect(m_ui->elementEditor, &ElementEditor::editSubcircuitRequested, this, [this](const QString &blobName, int icElementId) {
        if (m_currentTab) {
            openICInTab(blobName, icElementId, m_currentTab->scene()->icRegistry()->blob(blobName));
        }
    });
    connect(m_ui->elementEditor, &ElementEditor::embedSubcircuitRequested, this, &MainWindow::embedSelectedIC);
    connect(m_ui->elementEditor, &ElementEditor::extractToFileRequested, this, &MainWindow::extractSelectedIC);
    connect(m_ui->actionResetZoom,             &QAction::triggered,       this,                &MainWindow::on_actionResetZoom_triggered);
    connect(m_ui->actionRestart,               &QAction::triggered,       this,                &MainWindow::on_actionRestart_triggered);
    connect(m_ui->actionRotateLeft,            &QAction::triggered,       this,                &MainWindow::on_actionRotateLeft_triggered);
    connect(m_ui->actionRotateRight,           &QAction::triggered,       this,                &MainWindow::on_actionRotateRight_triggered);
    connect(m_ui->actionSave,                  &QAction::triggered,       this,                &MainWindow::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,               &QAction::triggered,       this,                &MainWindow::on_actionSaveAs_triggered);
    connect(m_ui->actionSelectAll,             &QAction::triggered,       this,                &MainWindow::on_actionSelectAll_triggered);
    connect(m_ui->actionShortcutsAndTips,      &QAction::triggered,       this,                &MainWindow::on_actionShortcuts_and_Tips_triggered);
    connect(m_ui->actionWaveform,              &QAction::triggered,       this,                &MainWindow::on_actionWaveform_triggered);
    connect(m_ui->actionWires,                 &QAction::triggered,       this,                &MainWindow::on_actionWires_triggered);
    connect(m_ui->actionZoomIn,                &QAction::triggered,       this,                &MainWindow::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,               &QAction::triggered,       this,                &MainWindow::on_actionZoomOut_triggered);
    connect(m_palette,                         &ElementPalette::addElementRequested, this, [this](QMimeData *mimeData) {
        if (m_currentTab) m_currentTab->scene()->addItem(mimeData);
    });
    connect(m_ui->pushButtonAddIC,             &QPushButton::clicked,     this,                &MainWindow::on_pushButtonAddIC_clicked);
    connect(m_ui->pushButtonRemoveIC,          &QPushButton::clicked,     this,                &MainWindow::on_pushButtonRemoveIC_clicked);
    connect(m_ui->pushButtonRemoveIC,          &TrashButton::removeICFile,this,                &MainWindow::removeICFile);
    connect(m_ui->pushButtonMakeSelfContained, &QPushButton::clicked,     this,                &MainWindow::makeSelfContained);
    connect(m_ui->actionMakeSelfContained,     &QAction::triggered,       this,                &MainWindow::makeSelfContained);

    // ICDropZone cross-section drag-and-drop
    connect(m_ui->dropZoneFileBased, &ICDropZone::extractByBlobNameRequested, this, &MainWindow::extractICByBlobName);
    connect(m_ui->dropZoneEmbedded, &ICDropZone::embedByFileRequested, this, &MainWindow::embedICByFile);

    // Embedded IC section buttons
    connect(m_ui->pushButtonAddEmbeddedIC, &QPushButton::clicked, this, [this] {
        if (!m_currentTab) return;
        QString fileName = FileDialogs::provider()->getOpenFileName(this, i18n("Select IC file to embed"), currentDir().absolutePath(), i18n("Panda files (*.panda)"));
        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            MessageDialog::warning(this, i18n("Could not read file: %1", file.errorString()), i18n("Error"));
            return;
        }
        QByteArray fileBytes = file.readAll();
        file.close();

        auto *scene = m_currentTab->scene();
        QString blobName = resolveUniqueBlobName(QFileInfo(fileName).baseName(), scene);
        if (blobName.isEmpty()) return;

        scene->receiveCommand(new RegisterBlobCommand(blobName, fileBytes, scene));
        m_palette->updateEmbeddedICList(scene);
    });
    connect(m_ui->pushButtonRemoveEmbeddedIC, &QPushButton::clicked, this, &MainWindow::on_pushButtonRemoveIC_clicked);
    connect(m_ui->pushButtonRemoveEmbeddedIC, &TrashButton::removeEmbeddedIC, this, [this](const QString &blobName) {
        if (m_currentTab) {
            m_currentTab->removeEmbeddedIC(blobName);
            m_palette->updateEmbeddedICList(m_currentTab->scene());
        }
    });

    // These edit actions always delegate to the current tab's scene, so they
    // never need to be rewired on tab switch.
    connectSceneAction(m_ui->actionClearSelection, &Scene::clearSelection);
    connectSceneAction(m_ui->actionCopy,           &Scene::copyAction);
    connectSceneAction(m_ui->actionCut,            &Scene::cutAction);
    connectSceneAction(m_ui->actionDelete,         &Scene::deleteAction);
    connectSceneAction(m_ui->actionPaste,          &Scene::pasteAction);
}

void MainWindow::connectSceneAction(QAction *action, void (Scene::*method)())
{
    connect(action, &QAction::triggered, this, [this, method] {
        if (m_currentTab) {
            (m_currentTab->scene()->*method)();
        }
    });
}

MainWindow::~MainWindow()
{
    disconnectTab();
}

void MainWindow::loadAutosaveFiles()
{
    QStringList autosaves(Settings::autosaveFiles());

    qCDebug(zero) << "All autosave files: " << autosaves;

    for (auto it = autosaves.begin(); it != autosaves.end();) {
        QFile file(*it);

        if (!file.exists()) {
            qCDebug(zero) << "Removing from config the autosave file that does not exist.";
            it = autosaves.erase(it);
            continue;
        }

        try {
            loadPandaFile(*it);
        } catch (const std::exception &e) {
            if (Application::interactiveMode) {
                MessageDialog::error(nullptr, e.what(), i18n("Error!"));
            }
            qCDebug(zero) << "Removing autosave file that is corrupted.";
            it = autosaves.erase(it);
            continue;
        }

        // Mark the newly loaded tab so it knows it came from an autosave,
        // causing it to prompt for a real save path on the next Ctrl+S.
        m_currentTab->setAutosaveFile();

        ++it;
    }

    Settings::setAutosaveFiles(autosaves);
}

void MainWindow::createNewTab()
{
    qCDebug(zero) << "Creating new workspace.";
    auto *workspace = new WorkSpace(this);

    connect(workspace, &WorkSpace::fileChanged, this, &MainWindow::setCurrentFile);

    workspace->view()->setFastMode(m_ui->actionFastMode->isChecked());
    workspace->scene()->updateTheme();

    qCDebug(zero) << "Adding tab. #tabs: " << m_ui->tab->count() << ", current tab: " << m_tabIndex;
    m_ui->tab->addTab(workspace, i18n("New Project"));
    sentryBreadcrumb("ui", QStringLiteral("Tab opened"));

    qCDebug(zero) << "Selecting the newly created tab.";
    m_ui->tab->setCurrentIndex(m_ui->tab->count() - 1);

    qCDebug(zero) << "Finished #tabs: " << m_ui->tab->count() << ", current tab: " << m_tabIndex;
}

void MainWindow::removeUndoRedoMenu()
{
    if (!m_currentTab) {
        return;
    }

    // The undo/redo actions are always inserted at positions 0 and 1 of menuEdit.
    // Any fewer entries means they were never added, so nothing to remove.
    const auto actions = m_ui->menuEdit->actions();
    if (actions.size() < 2) {
        return;
    }
    m_ui->menuEdit->removeAction(actions.at(0));
    m_ui->menuEdit->removeAction(actions.at(1));
}

void MainWindow::addUndoRedoMenu()
{
    if (!m_currentTab) {
        return;
    }

    auto *scene = m_currentTab->scene();
    if (!scene) {
        return;
    }

    const auto actions = m_ui->menuEdit->actions();
    if (actions.size() < 2) {
        return;
    }

    // Insert before position 0 then before the new position 1 so undo appears
    // first, redo second — above the separator that already exists in menuEdit.
    m_ui->menuEdit->insertAction(actions.at(0), scene->undoAction());
    m_ui->menuEdit->insertAction(m_ui->menuEdit->actions().at(1), scene->redoAction());
}

void MainWindow::setFastMode(const bool fastMode)
{
    m_ui->actionFastMode->setChecked(fastMode);

    if (m_currentTab) {
        m_currentTab->view()->setFastMode(fastMode);
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::save(const QString &fileName)
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->save(fileName);
    m_palette->updateICList(icListFile());
    m_ui->statusBar->showMessage(i18n("File saved successfully."), 4000);
}

void MainWindow::show()
{
    QMainWindow::show();

    qCDebug(zero) << "Checking for autosave file recovery.";
    loadAutosaveFiles();

    if (Application::interactiveMode) {
        auto *updateChecker = new UpdateChecker(this);
        connect(updateChecker, &UpdateChecker::updateAvailable, this, &MainWindow::showUpdateDialog);
        updateChecker->checkForUpdates();
    }
}

void MainWindow::showUpdateDialog(const QString &latestVersion, const QUrl &downloadUrl, const QUrl &releaseUrl)
{
    const bool hasDirectDownload = downloadUrl.isValid() && !downloadUrl.isEmpty();

#ifdef USE_KDE_FRAMEWORKS
    // Phase 7b: passive system notification instead of a modal dialog. The user
    // can dismiss without breaking flow; clicking an action triggers download
    // / opens the release page / records "skip this version".
    auto *notification = new KNotification(QStringLiteral("updateAvailable"));
    notification->setTitle(i18n("wiRedPanda Update Available"));
    notification->setText(hasDirectDownload
        ? i18n("Version %1 is available (you have %2). Click Download to save it.",
               latestVersion, QString::fromLatin1(APP_VERSION))
        : i18n("Version %1 is available (you have %2). Click View Release to open the release page.",
               latestVersion, QString::fromLatin1(APP_VERSION)));
    notification->setFlags(KNotification::Persistent);

    auto *primary = notification->addAction(hasDirectDownload ? i18n("Download") : i18n("View Release"));
    connect(primary, &KNotificationAction::activated, this,
            [this, latestVersion, downloadUrl, releaseUrl, hasDirectDownload] {
                if (hasDirectDownload) {
                    downloadUpdate(latestVersion, downloadUrl);
                } else {
                    QDesktopServices::openUrl(releaseUrl);
                    Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));
                }
            });

    auto *skipAction = notification->addAction(i18n("Skip This Version"));
    connect(skipAction, &KNotificationAction::activated, this, [latestVersion] {
        Settings::setUpdateCheckSkippedVersion(latestVersion);
    });

    connect(notification, &KNotification::closed, this, [] {
        Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));
    });

    notification->sendEvent();
    return;
#else
    QDialog dialog(this);
    dialog.setWindowTitle(i18n("Update Available"));
    dialog.setWindowModality(Qt::WindowModal);

    auto *layout = new QVBoxLayout(&dialog);

    auto *label = new QLabel(
        (hasDirectDownload
             ? i18n("<b>wiRedPanda %1 is available.</b><br><br>"
                  "You are currently running version %2.<br>"
                  "Click <b>Download</b> to save the new version to your computer.")
             : i18n("<b>wiRedPanda %1 is available.</b><br><br>"
                  "You are currently running version %2.<br>"
                  "Visit the release page to download the new version."))
            .arg(latestVersion, APP_VERSION),
        &dialog);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    layout->addWidget(label);

    auto *skipCheckBox = new QCheckBox(i18n("Don't notify me about this version again"), &dialog);
    layout->addWidget(skipCheckBox);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    auto *downloadButton = buttonBox->addButton(i18n("Download"), QDialogButtonBox::AcceptRole);
    connect(downloadButton, &QPushButton::clicked, &dialog, [&dialog] { dialog.accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    const bool accepted = dialog.exec() == QDialog::Accepted;

    if (skipCheckBox->isChecked()) {
        Settings::setUpdateCheckSkippedVersion(latestVersion);
    }

    if (accepted) {
        if (hasDirectDownload) {
            downloadUpdate(latestVersion, downloadUrl);
        } else {
            QDesktopServices::openUrl(releaseUrl);
            Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));
        }
    } else {
        /// User closed dialog without taking action — still record the check
        Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));
    }
#endif
}

#ifdef USE_KDE_FRAMEWORKS
void MainWindow::downloadCircuits()
{
    auto *dialog = new KNSWidgets::Dialog(u"wiredpanda.knsrc"_s, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::finished, this, [this, dialog](int) {
        const auto entries = dialog->changedEntries();
        for (const auto &entry : entries) {
            if (entry.status() != KNSCore::Entry::Installed) {
                continue;
            }
            const auto installedFiles = entry.installedFiles();
            for (const auto &file : installedFiles) {
                if (file.endsWith(u".panda"_s, Qt::CaseInsensitive)) {
                    loadPandaFile(file);
                }
            }
        }
    });
    dialog->open();
}
#endif

void MainWindow::downloadUpdate(const QString &latestVersion, const QUrl &url)
{
    const QString fileName = url.fileName();
    const QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath(fileName);

    auto *progress = new QProgressDialog(i18n("Downloading wiRedPanda %1…", latestVersion), i18n("Cancel"), 0, 100, this);
    progress->setWindowTitle(i18n("Downloading Update"));
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setValue(0);

    auto *network = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = network->get(request);

    connect(reply, &QNetworkReply::downloadProgress, progress, [progress](qint64 received, qint64 total) {
        if (total > 0) {
            progress->setValue(static_cast<int>(received * 100 / total));
        }
    });

    connect(progress, &QProgressDialog::canceled, reply, &QNetworkReply::abort);

    connect(reply, &QNetworkReply::finished, this, [this, reply, progress, savePath] {
        progress->close();
        progress->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                MessageDialog::warning(this, i18n("Could not download the update:\n%1", reply->errorString()), i18n("Download Failed"));
            }
            reply->deleteLater();
            return;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            MessageDialog::warning(this, i18n("Could not save the file:\n%1", savePath), i18n("Download Failed"));
            reply->deleteLater();
            return;
        }
        file.write(reply->readAll());
        file.close();
        reply->deleteLater();

        Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));
        MessageDialog::information(this,
            i18n("wiRedPanda has been downloaded to:\n%1", savePath),
            i18n("Download Complete"));
    });
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

int MainWindow::closeTabAnyway()
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(i18n("File not saved. Close tab anyway?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}

int MainWindow::confirmSave(const bool multiple)
{
    QMessageBox msgBox;
    msgBox.setParent(this);

    // When closing all tabs at once, offer "Yes to All" / "No to All" to avoid
    // repeated per-file prompts.
    if (multiple) {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
    } else {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    }

    const QString fileName = currentFile().fileName().isEmpty() ? i18n("New Project") : currentFile().fileName();

    msgBox.setText(fileName + i18n(" has been modified.\nDo you want to save your changes?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

void MainWindow::on_actionNew_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("New project"));
    createNewTab();
}

void MainWindow::on_actionWires_triggered(const bool checked)
{
    sentryBreadcrumb("ui", QStringLiteral("Wires: %1").arg(checked));
    if (m_currentTab) {
        m_currentTab->scene()->showWires(checked);
    }
}

void MainWindow::on_actionRotateRight_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Rotate right"));
    if (m_currentTab) {
        m_currentTab->scene()->rotateRight();
    }
}

void MainWindow::on_actionRotateLeft_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Rotate left"));
    if (m_currentTab) {
        m_currentTab->scene()->rotateLeft();
    }
}

void MainWindow::loadPandaFile(const QString &fileName)
{
    createNewTab();
    qCDebug(zero) << "Loading in editor.";
    m_currentTab->load(fileName);
    // Tighten the scene rect to the loaded items immediately so subsequent
    // interactions (selection, drag release) don't cause a viewport jump.
    m_currentTab->scene()->resizeScene();
    m_palette->updateICList(icListFile());
    m_palette->updateEmbeddedICList(m_currentTab->scene());
    m_ui->statusBar->showMessage(i18n("File loaded successfully."), 4000);
}

void MainWindow::openICInTab(const QString &blobName, int icElementId, const QByteArray &blob)
{
    if (!m_currentTab) {
        return;
    }

    // Check if this blob is already being edited in a tab
    for (int i = 0; i < m_ui->tab->count(); ++i) {
        auto *ws = qobject_cast<WorkSpace *>(m_ui->tab->widget(i));
        if (ws && ws->isInlineIC() && ws->inlineBlobName() == blobName
            && ws->parentWorkspace() == m_currentTab) {
            m_ui->tab->setCurrentIndex(i);
            return;
        }
    }

    auto *parentWorkspace = m_currentTab;

    createNewTab();

    m_currentTab->loadFromBlob(blob, parentWorkspace, icElementId, parentWorkspace->scene()->contextDir());
    // Tighten the scene rect to the loaded items immediately so subsequent
    // interactions (selection, drag release) don't cause a viewport jump.
    m_currentTab->scene()->resizeScene();

    // Hide management buttons for inline tabs (they use currentFile/currentDir which are empty)
    setICButtonsVisible(false);

    // Set tab title
    int tabIndex = m_ui->tab->indexOf(m_currentTab);
    m_ui->tab->setTabText(tabIndex, "[" + blobName + "]");

    m_palette->updateICList(icListFile());
    m_palette->updateEmbeddedICList(m_currentTab->scene());

    // Connect child tab's save signal to parent
    connect(m_currentTab, &WorkSpace::icBlobSaved, parentWorkspace, &WorkSpace::onChildICBlobSaved);
}

void MainWindow::on_actionOpen_triggered()
{
    sentryBreadcrumb("file", QStringLiteral("Open file dialog"));
#ifdef Q_OS_WASM
    auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent) {
        if (!fileName.isEmpty()) {
            // Write file content to a temporary file
            QFile file(fileName);
            file.open(QIODevice::WriteOnly);
            file.write(fileContent);
            file.close();
            loadPandaFile(fileName);
        }
    };
    QFileDialog::getOpenFileContent("Panda files (*.panda)", fileContentReady);
#else
    const QString path = currentFile().exists() ? "" : "./Examples";
    const QString fileName = FileDialogs::provider()->getOpenFileName(this, i18n("Open File"), path, i18n("Panda files (*.panda)"));

    if (fileName.isEmpty()) {
        return;
    }

    loadPandaFile(fileName);
#endif
}

void MainWindow::on_actionSave_triggered()
{
    sentryBreadcrumb("file", QStringLiteral("Save"));
    if (!m_currentTab) {
        return;
    }

    // Inline IC tabs serialize to a blob and emit to parent — no file dialog needed.
    if (m_currentTab->isInlineIC()) {
        save(QString());
        return;
    }

#ifdef Q_OS_WASM
    on_actionSaveAs_triggered();
#else
    // TODO: if current file is autosave ask for filename

    // If the project has never been saved, fall through to a Save-As dialog.
    QString fileName = currentFile().absoluteFilePath();

    if (fileName.isEmpty()) {
        fileName = FileDialogs::provider()->getSaveFileName(this, i18n("Save File as ..."), QString(), i18n("Panda files (*.panda)")).fileName;

        if (fileName.isEmpty()) {
            return;
        }

        ensureFileExtension(fileName, ".panda");
    }

    save(fileName);
#endif
}

void MainWindow::on_actionSaveAs_triggered()
{
    sentryBreadcrumb("file", QStringLiteral("Save as"));
    if (!m_currentTab) {
        return;
    }

#ifdef Q_OS_WASM
    // Save to a temporary file in the virtual FS, then offer it as a browser download.
    const QString tmpPath = "/tmp/wiredpanda_save.panda";
    save(tmpPath);

    QFile file(tmpPath);
    if (file.open(QIODevice::ReadOnly)) {
        const QByteArray content = file.readAll();
        file.close();

        QString suggestedName = currentFile().fileName();
        if (suggestedName.isEmpty()) {
            suggestedName = "circuit.panda";
        }
        QFileDialog::saveFileContent(content, suggestedName);
    }
#else
    QString fileName = FileDialogs::provider()->getSaveFileName(this, i18n("Save File as ..."), currentFile().absoluteFilePath(), i18n("Panda files (*.panda)")).fileName;

    if (fileName.isEmpty()) {
        return;
    }

    ensureFileExtension(fileName, ".panda");

    save(fileName);
#endif
}

void MainWindow::on_actionAbout_triggered()
{
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
           "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
            .arg(QApplication::applicationVersion()));
}

void MainWindow::on_actionShortcuts_and_Tips_triggered()
{
    MessageDialog::information(this,
        i18n("<h1>Canvas Shortcuts</h1>"
           "<ul style=\"list-style:none;\">"
           "<li> Ctrl+= : Zoom in </li>"
           "<li> Ctrl+- : Zoom out </li>"
           "<li> Ctrl+1 : Hide/Show wires </li>"
           "<li> Ctrl+2 : Hide/Show gates </li>"
           "<li> Ctrl+F : Search elements </li>"
           "<li> Ctrl+W : Open beWaveDolphin </li>"
           "<li> Ctrl+S : Save project </li>"
           "<li> Ctrl+Q : Exit wiRedPanda </li>"
           "<li> F5 : Start/Pause simulation </li>"
           "<li> [ : Previous primary element property </li>"
           "<li> ] : Next primary element property </li>"
           "<li> { : Previous secondary element property </li>"
           "<li> } : Next secondary element property </li>"
           "<li> &lt; : Morph to previous element </li>"
           "<li> &gt; : Morph to next element </li>"
           "</ul>"

           "<h1>General Tips</h1>"
           "<p>Double-click on a wire to create a node</p>"
        ),
        i18n("Shortcuts and Tips"));
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionReportTranslationError_triggered()
{
    QDesktopServices::openUrl(QUrl("https://hosted.weblate.org/projects/wiredpanda/wiredpanda"));
}

bool MainWindow::closeFiles()
{
    // Close from last to first so that tab indices stay stable during iteration.
    while (m_ui->tab->count() != 0) {
        if (!closeTab(m_ui->tab->count() - 1)) {
            return false;
        }
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool closeWindow = false;

    // If nothing is modified, ask once before exiting so the user can't
    // accidentally quit with a keyboard shortcut.  If there are unsaved changes,
    // delegate to closeFiles() which prompts per-tab.
    if (!hasModifiedFiles()) {
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
    } else if (closeFiles()) {
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

void MainWindow::ensureFileExtension(QString &fileName, const QString &extension)
{
    if (!fileName.endsWith(extension)) {
        fileName.append(extension);
    }
}

IC *MainWindow::getSelectedIC() const
{
    if (!m_currentTab) {
        return nullptr;
    }

    const auto selected = m_currentTab->scene()->selectedElements();
    if (selected.isEmpty()) {
        return nullptr;
    }

    if (selected.first()->elementType() != ElementType::IC) {
        return nullptr;
    }

    return static_cast<IC *>(selected.first());
}

void MainWindow::setICButtonsVisible(bool visible)
{
    m_ui->pushButtonAddIC->setVisible(visible);
    m_ui->pushButtonRemoveIC->setVisible(visible);
    m_ui->pushButtonMakeSelfContained->setVisible(visible);
}

bool MainWindow::hasModifiedFiles()
{
    const QStringList autosaves = Settings::autosaveFiles();

    const auto workspaces = m_ui->tab->findChildren<WorkSpace *>();

    for (auto *workspace : workspaces) {
        auto *scene = workspace->scene();
        if (!scene) {
            continue;
        }

        auto *undoStack = scene->undoStack();
        if (!undoStack) {
            continue;
        }

        // An un-clean undo stack means uncommitted edits since the last save.
        if (!undoStack->isClean()) {
            return true;
        }

        // An autosave file that is still in the list has not been explicitly
        // saved by the user yet and should be treated as modified.
        const QString fileName = workspace->fileInfo().fileName();

        if (!fileName.isEmpty() && autosaves.contains(fileName)) {
            return true;
        }
    }

    return false;
}

QFileInfo MainWindow::currentFile() const
{
    return m_currentTab ? m_currentTab->fileInfo() : QFileInfo();
}

QDir MainWindow::currentDir() const
{
    return m_currentTab ? m_currentTab->fileInfo().absoluteDir() : QDir();
}

QFileInfo MainWindow::icListFile() const
{
    // Walk up the parent workspace chain to find the root file on disk.
    // Inline IC workspaces have no file of their own.
    auto *ws = m_currentTab;
    while (ws && ws->isInlineIC() && ws->parentWorkspace()) {
        ws = ws->parentWorkspace();
    }
    if (ws) {
        return ws->fileInfo();
    }
    return currentFile();
}

void MainWindow::setCurrentFile(const QFileInfo &fileInfo)
{
    // Find the tab belonging to the workspace that emitted the signal.
    // The sender may differ from m_currentTab (e.g. a parent workspace emitting
    // fileChanged while an inline IC child tab is active).
    auto *senderWs = qobject_cast<WorkSpace *>(sender());
    if (!senderWs) {
        senderWs = m_currentTab;
    }
    if (!senderWs) {
        return;
    }

    const int tabIndex = m_ui->tab->indexOf(senderWs);
    if (tabIndex < 0) {
        return;
    }

    // Inline IC tabs use "[blobName]" as title — never the parent filename.
    QString text;
    if (senderWs->isInlineIC()) {
        text = "[" + senderWs->inlineBlobName() + "]";
    } else {
        text = fileInfo.exists() ? fileInfo.fileName() : i18n("New Project");
    }

    // Append an asterisk to the tab title to indicate unsaved changes,
    // following the common editor convention.
    auto *scene = senderWs->scene();
    if (scene) {
        auto *undoStack = scene->undoStack();
        if (undoStack && !undoStack->isClean()) {
            text += "*";
        }
    }

    m_ui->tab->setTabText(tabIndex, text);

    // Only update tooltip and recent files for file-backed tabs.
    if (!senderWs->isInlineIC()) {
        m_ui->tab->setTabToolTip(tabIndex, fileInfo.absoluteFilePath());
        qCDebug(zero) << "Adding file to recent files.";
        emit addRecentFile(fileInfo.absoluteFilePath());
    }
}

void MainWindow::on_actionSelectAll_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Select all"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->scene()->selectAll();
}

bool MainWindow::closeTab(const int tabIndex)
{
    qCDebug(zero) << "Closing tab " << tabIndex + 1 << ", #tabs: " << m_ui->tab->count();
    // Activate the tab being closed so m_currentTab reflects the right workspace
    // before we inspect its undo stack.
    m_ui->tab->setCurrentIndex(tabIndex);

    qCDebug(zero) << "Checking if needs to save file.";

    bool needsSave = false;
    if (m_currentTab) {
        auto *scene = m_currentTab->scene();
        if (scene) {
            auto *undoStack = scene->undoStack();
            needsSave = undoStack && !undoStack->isClean();
        }
    }

    if (needsSave) {
        const int selectedButton = confirmSave(false);

        if (selectedButton == QMessageBox::Cancel) {
            return false;
        }

        if (selectedButton == QMessageBox::Yes) {
            try {
                save();
            } catch (const std::exception &e) {
                MessageDialog::error(this, e.what(), i18n("Error"));

                // If saving failed ask whether to discard and close anyway.
                if (closeTabAnyway() == QMessageBox::No) {
                    return false;
                }
            }
        }
    }

    qCDebug(zero) << "Deleting tab.";
    m_currentTab->deleteLater();
    sentryBreadcrumb("ui", QStringLiteral("Tab closed"));
    m_ui->tab->removeTab(tabIndex);

    qCDebug(zero) << "Closed tab " << tabIndex << ", #tabs: " << m_ui->tab->count() << ", current tab: " << m_tabIndex;

    return true;
}

void MainWindow::disconnectTab()
{
    // Called before switching away from a tab.  Tear down all connections that
    // route scene signals into shared UI elements, and stop the simulation so it
    // doesn't keep running in the background consuming CPU.
    if (!m_currentTab) {
        return;
    }

    m_ui->elementEditor->setScene(nullptr);

    qCDebug(zero) << "Stopping simulation.";
    m_currentTab->simulation()->stop();

    qCDebug(zero) << "Disconnecting zoom and simulation signals from UI.";
    disconnect(m_currentTab->view(),       &GraphicsView::zoomChanged,        this, &MainWindow::zoomChanged);
    disconnect(m_currentTab->simulation(), &Simulation::simulationWarning,    this, nullptr);

    qCDebug(zero) << "Disconnecting scene shortcuts from previous tab.";
    disconnect(m_prevMainPropShortcut,  nullptr, m_currentTab->scene(), nullptr);
    disconnect(m_nextMainPropShortcut,  nullptr, m_currentTab->scene(), nullptr);
    disconnect(m_prevSecndPropShortcut, nullptr, m_currentTab->scene(), nullptr);
    disconnect(m_nextSecndPropShortcut, nullptr, m_currentTab->scene(), nullptr);
    disconnect(m_changePrevElmShortcut, nullptr, m_currentTab->scene(), nullptr);
    disconnect(m_changeNextElmShortcut, nullptr, m_currentTab->scene(), nullptr);
    disconnect(m_currentTab->scene(), &Scene::icOpenRequested, this, nullptr);
    disconnect(m_currentTab->scene(), &Scene::openTruthTableRequested, this, nullptr);
    disconnect(m_currentTab->scene()->undoStack(), &QUndoStack::indexChanged, this, nullptr);

    qCDebug(zero) << "Removing undo and redo actions from UI menu.";
    removeUndoRedoMenu();
}

void MainWindow::connectTab()
{
    qCDebug(zero) << "Connecting undo and redo functions to UI menu.";
    addUndoRedoMenu();

    m_palette->updateICList(icListFile());
    m_palette->updateEmbeddedICList(m_currentTab->scene());

    // Hide management buttons for inline IC tabs (they use currentFile/currentDir which are empty)
    setICButtonsVisible(!m_currentTab->isInlineIC());

    qCDebug(zero) << "Connecting current tab to element editor menu in UI.";
    m_ui->elementEditor->setScene(m_currentTab->scene());

    connect(m_currentTab->view(),       &GraphicsView::zoomChanged, this,                  &MainWindow::zoomChanged);
    connect(m_currentTab->simulation(), &Simulation::simulationWarning, this, [this](const QString &msg) {
        m_ui->statusBar->showMessage(msg, 8000);
    });
    connect(m_currentTab->scene()->undoStack(), &QUndoStack::indexChanged, this, [this] {
        if (m_currentTab) {
            m_palette->updateEmbeddedICList(m_currentTab->scene());
        }
    });

    connect(m_prevMainPropShortcut,  &QShortcut::activated, m_currentTab->scene(), &Scene::prevMainPropShortcut);
    connect(m_nextMainPropShortcut,  &QShortcut::activated, m_currentTab->scene(), &Scene::nextMainPropShortcut);
    connect(m_prevSecndPropShortcut, &QShortcut::activated, m_currentTab->scene(), &Scene::prevSecndPropShortcut);
    connect(m_nextSecndPropShortcut, &QShortcut::activated, m_currentTab->scene(), &Scene::nextSecndPropShortcut);
    connect(m_changePrevElmShortcut, &QShortcut::activated, m_currentTab->scene(), &Scene::prevElm);
    connect(m_changeNextElmShortcut, &QShortcut::activated, m_currentTab->scene(), &Scene::nextElm);
    connect(m_currentTab->scene(), &Scene::openTruthTableRequested, this, [this] {
        m_ui->elementEditor->truthTable();
    });
    connect(m_currentTab->scene(), &Scene::icOpenRequested, this, [this](int elementId, const QString &blobName, const QString &filePath) {
        if (!blobName.isEmpty()) {
            if (m_currentTab) {
                openICInTab(blobName, elementId, m_currentTab->scene()->icRegistry()->blob(blobName));
            }
        } else if (!filePath.isEmpty()) {
            loadPandaFile(filePath);
        }
    });

    if (m_ui->actionPlay->isChecked()) {
        qCDebug(zero) << "Restarting simulation.";
        m_currentTab->simulation()->start();
        // Clear selection so the element editor doesn't show stale data from the
        // previously active tab.
        m_currentTab->scene()->clearSelection();
    }

    m_currentTab->view()->setFastMode(m_ui->actionFastMode->isChecked());
    // Synchronise zoom button state to the newly visible tab's zoom level.
    m_ui->actionZoomIn->setEnabled(m_currentTab->view()->canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_currentTab->view()->canZoomOut());

    // Synchronise the mute button state to the newly visible tab's mute intent.
    const bool muted = m_currentTab->simulation()->isUserMuted();
    m_ui->actionMute->setChecked(muted);
    m_ui->actionMute->setText(muted ? i18n("Unmute") : i18n("Mute"));

}

WorkSpace *MainWindow::currentTab() const
{
    return m_currentTab;
}

void MainWindow::tabChanged(const int newTabIndex)
{
    sentryBreadcrumb("ui", QStringLiteral("Tab changed to index %1").arg(newTabIndex));
    disconnectTab(); // disconnect previously selected tab
    m_tabIndex = newTabIndex;
    // Hide the editor panel during the transition; connectTab() will restore it
    // once the new scene's selection is known.
    m_ui->elementEditor->hide();

    if (newTabIndex == -1) {
        // All tabs were closed; reset state.
        m_currentTab = nullptr;
        m_palette->updateICList(QFileInfo());
        m_palette->updateEmbeddedICList(nullptr);
        return;
    }

    m_currentTab = qobject_cast<WorkSpace *>(m_ui->tab->currentWidget());
    qCDebug(zero) << "Selecting tab: " << newTabIndex;
    connectTab();
    qCDebug(zero) << "New tab selected. Dolphin fileName: " << m_currentTab->dolphinFileName();
}

void MainWindow::on_actionReloadFile_triggered()
{
    sentryBreadcrumb("file", QStringLiteral("Reload file"));
    if (!currentFile().exists() || !m_currentTab) {
        return;
    }

    const QString file = currentFile().absoluteFilePath();

    closeTab(m_tabIndex);
    loadPandaFile(file);
}

void MainWindow::on_actionGates_triggered(const bool checked)
{
    sentryBreadcrumb("ui", QStringLiteral("Gates: %1").arg(checked));
    if (!m_currentTab) {
        return;
    }

    // Wire visibility depends on gates being visible: if gates are hidden, wires
    // make no sense and should be hidden too.  Re-enable the wire toggle only when
    // gates are shown so the user can't end up with floating wires.
    m_ui->actionWires->setEnabled(checked);
    m_currentTab->scene()->showWires(checked ? m_ui->actionWires->isChecked() : checked);
    m_currentTab->scene()->showGates(checked);
}

void MainWindow::exportToArduino(QString fileName)
{
    if (!m_currentTab) {
        return;
    }

    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    auto elements = m_currentTab->scene()->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The .panda file is empty.");
    }

    // Pause the simulation while generating code to avoid data races between
    // the simulation timer and the code generator reading element state.
    SimulationBlocker simulationBlocker(m_currentTab->simulation());

    ensureFileExtension(fileName, ".ino");

    ArduinoCodeGen arduino(QDir::home().absoluteFilePath(fileName), elements);
    arduino.generate();
    m_ui->statusBar->showMessage(i18n("Arduino code successfully generated."), 4000);

    qCDebug(zero) << "Arduino code successfully generated.";
}

void MainWindow::exportToSystemVerilog(QString fileName)
{
    if (!m_currentTab) {
        return;
    }

    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    auto elements = m_currentTab->scene()->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION("The .panda file is empty.");
    }

    SimulationBlocker simulationBlocker(m_currentTab->simulation());

    ensureFileExtension(fileName, ".sv");

    SystemVerilogCodeGen verilog(QDir::home().absoluteFilePath(fileName), elements);
    verilog.generate();
    m_ui->statusBar->showMessage(i18n("SystemVerilog code successfully generated."), 4000);

    qCDebug(zero) << "SystemVerilog code successfully generated.";
}

void MainWindow::exportToWaveFormFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }

    auto *bewavedDolphin = new BewavedDolphin(m_currentTab->scene(), false, this);
    bewavedDolphin->createWaveform(fileName);
    bewavedDolphin->print();
}

void MainWindow::exportToWaveFormTerminal()
{
    auto *bewavedDolphin = new BewavedDolphin(m_currentTab->scene(), false, this);
    bewavedDolphin->createWaveform();
    bewavedDolphin->print();
}

void MainWindow::on_actionExportToArduino_triggered()
{
    sentryBreadcrumb("export", QStringLiteral("Export to Arduino"));
    if (!m_currentTab) {
        return;
    }

    QString path;

    if (currentFile().exists()) {
        path = currentFile().absolutePath();
    }

    const QString fileName = FileDialogs::provider()->getSaveFileName(this, i18n("Generate Arduino Code"), path, i18n("Arduino file (*.ino)")).fileName;

    if (!fileName.isEmpty()) {
        exportToArduino(fileName);
    }
}

void MainWindow::on_actionExportToSystemVerilog_triggered()
{
    sentryBreadcrumb("export", QStringLiteral("Export to SystemVerilog"));
    if (!m_currentTab) {
        return;
    }

    QString path;

    if (currentFile().exists()) {
        path = currentFile().absolutePath();
    }

    const QString fileName = FileDialogs::provider()->getSaveFileName(this, i18n("Generate SystemVerilog Code"), path, i18n("SystemVerilog file (*.sv)")).fileName;

    if (!fileName.isEmpty()) {
        exportToSystemVerilog(fileName);
    }
}

void MainWindow::on_actionZoomIn_triggered() const
{
    sentryBreadcrumb("ui", QStringLiteral("Zoom in"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->view()->zoomIn();
}

void MainWindow::on_actionZoomOut_triggered() const
{
    sentryBreadcrumb("ui", QStringLiteral("Zoom out"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->view()->zoomOut();
}

void MainWindow::on_actionResetZoom_triggered() const
{
    sentryBreadcrumb("ui", QStringLiteral("Zoom reset"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->view()->resetZoom();
}

void MainWindow::zoomChanged()
{
    if (!m_currentTab) {
        return;
    }

    m_ui->actionZoomIn->setEnabled(m_currentTab->view()->canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_currentTab->view()->canZoomOut());
}

#ifndef USE_KDE_FRAMEWORKS
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
#endif // USE_KDE_FRAMEWORKS

void MainWindow::on_actionExportToPdf_triggered()
{
    sentryBreadcrumb("export", QStringLiteral("Export to PDF"));
    if (!m_currentTab) {
        return;
    }

    // De-select elements so their selection handles don't appear in the export.
    m_currentTab->scene()->clearSelection();

    const QString path    = currentFile().exists() ? currentFile().absolutePath() : QString();
    QString pdfFile = FileDialogs::provider()->getSaveFileName(this, i18n("Export to PDF"), path, i18n("PDF files (*.pdf)")).fileName;

    if (pdfFile.isEmpty()) {
        return;
    }

    if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
        pdfFile.append(".pdf");
    }

    CircuitExporter::renderToPdf(m_currentTab->scene(), pdfFile);
    m_ui->statusBar->showMessage(i18n("Exported file successfully."), 4000);
    QDesktopServices::openUrl(QUrl::fromLocalFile(pdfFile));
}

void MainWindow::on_actionExportToImage_triggered()
{
    sentryBreadcrumb("export", QStringLiteral("Export to image"));
    if (!m_currentTab) {
        return;
    }

    // De-select elements so their selection handles don't appear in the export.
    m_currentTab->scene()->clearSelection();

    const QString path    = currentFile().exists() ? currentFile().absolutePath() : QString();
    QString pngFile = FileDialogs::provider()->getSaveFileName(this, i18n("Export to Image"), path, i18n("PNG files (*.png)")).fileName;

    if (pngFile.isEmpty()) {
        return;
    }

    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }

    CircuitExporter::renderToImage(m_currentTab->scene(), pngFile);
    m_ui->statusBar->showMessage(i18n("Exported file successfully."), 4000);
    QDesktopServices::openUrl(QUrl::fromLocalFile(pngFile));
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
    if (!m_currentTab) {
        return;
    }

    auto *simulation = m_currentTab->simulation();

    // The action is checkable; its toggled(bool) signal drives start/stop directly.
    checked ? simulation->start() : simulation->stop();
}

void MainWindow::on_actionRestart_triggered()
{
    sentryBreadcrumb("simulation", QStringLiteral("Simulation restart"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->simulation()->restart();
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
    sentryBreadcrumb("ui", QStringLiteral("Fast mode: %1").arg(checked));
    setFastMode(checked);
    Settings::setFastMode(checked);
}

void MainWindow::on_actionWaveform_triggered()
{
    if (!m_currentTab) {
        return;
    }

    sentryBreadcrumb("ui", QStringLiteral("Waveform dialog opened"));
    qCDebug(zero) << "BD fileName: " << m_currentTab->dolphinFileName();
    auto *bewavedDolphin = new BewavedDolphin(m_currentTab->scene(), true, this);
    bewavedDolphin->createWaveform(m_currentTab->dolphinFileName());
    bewavedDolphin->show();
}

void MainWindow::on_actionLightTheme_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Theme: light"));
    ThemeManager::setTheme(Theme::Light);
}

void MainWindow::on_actionDarkTheme_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Theme: dark"));
    ThemeManager::setTheme(Theme::Dark);
}

void MainWindow::on_actionSystemTheme_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Theme: system"));
    ThemeManager::setTheme(Theme::System);
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
    sentryBreadcrumb("ui", QStringLiteral("Flip horizontal"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->scene()->flipHorizontally();
}

void MainWindow::on_actionFlipVertically_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Flip vertical"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->scene()->flipVertically();
}

QString MainWindow::dolphinFileName()
{
    if (!m_currentTab) {
        return {};
    }

    return m_currentTab->dolphinFileName();
}

void MainWindow::setDolphinFileName(const QString &fileName)
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->setDolphinFileName(fileName);
}

void MainWindow::on_actionFullscreen_triggered()
{
    sentryBreadcrumb("ui", QStringLiteral("Fullscreen toggled"));
    isFullScreen() ? showNormal() : showFullScreen();
}

void MainWindow::on_actionMute_triggered(const bool checked)
{
    sentryBreadcrumb("ui", QStringLiteral("Mute toggled"));
    if (!m_currentTab) {
        return;
    }

    m_currentTab->simulation()->setUserMuted(checked);
    m_ui->actionMute->setText(checked ? i18n("Unmute") : i18n("Mute"));
}

void MainWindow::on_actionLabelsUnderIcons_triggered(const bool checked)
{
    sentryBreadcrumb("ui", QStringLiteral("Labels under icons: %1").arg(checked));
    m_ui->mainToolBar->setToolButtonStyle(checked ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    Settings::setLabelsUnderIcons(checked);
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

void MainWindow::on_pushButtonAddIC_clicked()
{
    sentryBreadcrumb("ic", QStringLiteral("Add IC"));
    if (!m_currentTab) {
        return;
    }

    // The IC list is directory-relative.  If the project hasn't been saved yet
    // we don't have a directory to copy into, so require a save first.
    if (!m_currentTab->fileInfo().isReadable()) {
        throw PANDACEPTION("Save file first.");
    }

    const QString selectedFile = FileDialogs::provider()->getOpenFileName(this, i18n("Open File"), QString(), i18n("Panda (*.panda)"));

    if (selectedFile.isEmpty()) {
        return;
    }

    const QStringList files = {selectedFile};

    MessageDialog::information(this, i18n("Selected files (and their dependencies) will be copied to the current project folder."), i18n("Info"));

    // Copy the chosen .panda file (and any ICs it depends on transitively)
    // into the project's directory so that relative paths work when reopened.
    for (const auto &file : files) {
        QFileInfo destPath(currentDir().absolutePath() + "/" + QFileInfo(file).fileName());
        Serialization::copyPandaFile(QFileInfo(file), destPath);
    }

    m_palette->updateICList(icListFile());
}

void MainWindow::on_pushButtonRemoveIC_clicked()
{
    sentryBreadcrumb("ic", QStringLiteral("Remove IC"));
    MessageDialog::information(this, i18n("Drag here to remove."), i18n("Info"));
}

void MainWindow::removeICFile(const QString &icFileName)
{
    if (!m_currentTab) {
        return;
    }

    // Stop simulation so IC elements aren't updated while we're deleting them.
    SimulationBlocker blocker(m_currentTab->simulation());

    auto elements = m_currentTab->scene()->elements();

    // Iterate in reverse so that removing items doesn't invalidate subsequent
    // iterators (the list is a copy, but the scene's internal container may shift).
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        if ((*it)->elementType() == ElementType::IC && (*it)->label().append(".panda").toLower() == icFileName) {
            m_currentTab->scene()->removeItem(*it);
            delete *it;
        }
    }

    QFile file(currentDir().absolutePath() + "/" + icFileName);

    if (!file.remove()) {
        throw PANDACEPTION("Error removing file: %1", file.errorString());
    }

    m_palette->updateICList(icListFile());
    // Auto-save after removal so the scene no longer references the deleted file.
    on_actionSave_triggered();
}

QString MainWindow::resolveUniqueBlobName(const QString &initialName, Scene *scene)
{
    auto *reg = scene->icRegistry();
    QString blobName = reg->uniqueBlobName(initialName.trimmed());

    // If the auto-generated name differs from the initial, let the user confirm or override
    if (blobName != initialName.trimmed()) {
        bool ok = false;
        blobName = QInputDialog::getText(this,
            i18n("Name Collision"),
            i18n("An embedded IC named \"%1\" already exists.\nSuggested name:", initialName.trimmed()),
            QLineEdit::Normal, blobName, &ok);
        blobName = blobName.trimmed();
        if (!ok || blobName.isEmpty()) {
            return {};
        }
        // Re-check in case the user typed a name that also collides
        if (reg->hasBlob(blobName)) {
            blobName = reg->uniqueBlobName(blobName);
        }
    }
    return blobName;
}

void MainWindow::embedSelectedIC()
{
    sentryBreadcrumb("ic", QStringLiteral("Embed IC"));
    auto *firstIC = getSelectedIC();
    if (!firstIC || firstIC->file().isEmpty()) {
        return;
    }

    auto *scene = m_currentTab->scene();

    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        MessageDialog::warning(this, i18n("Please save the project first so ICs can be resolved."), i18n("Error"));
        return;
    }

    QString blobName = resolveUniqueBlobName(QFileInfo(firstIC->file()).baseName(), scene);
    if (blobName.isEmpty()) {
        return;
    }

    QFile file(QDir(contextDir).absoluteFilePath(firstIC->file()));
    if (!file.open(QIODevice::ReadOnly)) {
        MessageDialog::warning(this, i18n("Could not read IC file: %1", file.errorString()), i18n("Error"));
        return;
    }
    QByteArray fileBytes = file.readAll();
    file.close();

    scene->icRegistry()->embedICsByFile(firstIC->file(), fileBytes, blobName);
    m_palette->updateEmbeddedICList(scene);
    m_ui->statusBar->showMessage(i18n("IC embedded successfully."), 4000);
}

void MainWindow::extractSelectedIC()
{
    sentryBreadcrumb("ic", QStringLiteral("Extract IC"));
    auto *firstIC = getSelectedIC();
    if (!firstIC || !firstIC->isEmbedded()) {
        return;
    }

    auto *scene = m_currentTab->scene();
    const QString blobName = firstIC->blobName();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        MessageDialog::warning(this, i18n("Please save the project first."), i18n("Error"));
        return;
    }

    const QString suggestion = QDir(contextDir).absoluteFilePath(blobName + ".panda");
    QString fileName = FileDialogs::provider()->getSaveFileName(this, i18n("Extract IC to file..."), suggestion, i18n("Panda files (*.panda)")).fileName;

    if (fileName.isEmpty()) {
        return;
    }

    ensureFileExtension(fileName, ".panda");

    scene->icRegistry()->extractToFile(blobName, fileName);
    m_palette->updateICList(icListFile());
    m_palette->updateEmbeddedICList(scene);
    m_ui->statusBar->showMessage(i18n("IC extracted to %1", fileName), 4000);
}

void MainWindow::embedICByFile(const QString &fileName)
{
    if (!m_currentTab) {
        return;
    }

    auto *scene = m_currentTab->scene();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        MessageDialog::warning(this, i18n("Please save the project first so ICs can be resolved."), i18n("Error"));
        return;
    }

    const QString absolutePath = QDir(contextDir).absoluteFilePath(fileName);
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        MessageDialog::warning(this, i18n("Could not read IC file: %1", file.errorString()), i18n("Error"));
        return;
    }
    QByteArray fileBytes = file.readAll();
    file.close();

    QString blobName = resolveUniqueBlobName(QFileInfo(absolutePath).baseName(), scene);
    if (blobName.isEmpty()) {
        return;
    }

    auto *reg = scene->icRegistry();
    if (reg->embedICsByFile(absolutePath, fileBytes, blobName) == 0) {
        // No existing instances — register the blob only; don't add to scene.
        scene->receiveCommand(new RegisterBlobCommand(blobName, fileBytes, scene));
    }

    m_palette->updateEmbeddedICList(scene);
    m_ui->statusBar->showMessage(i18n("IC embedded successfully."), 4000);
}

void MainWindow::extractICByBlobName(const QString &blobName)
{
    if (!m_currentTab) {
        return;
    }

    auto *scene = m_currentTab->scene();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        MessageDialog::warning(this, i18n("Please save the project first."), i18n("Error"));
        return;
    }

    // Find any embedded IC with this blobName to get the blob data
    auto *reg = scene->icRegistry();
    if (!reg->hasBlob(blobName)) {
        return;
    }

    const QString suggestion = QDir(contextDir).absoluteFilePath(blobName + ".panda");
    QString fileName = FileDialogs::provider()->getSaveFileName(this, i18n("Extract IC to file..."), suggestion, i18n("Panda files (*.panda)")).fileName;

    if (fileName.isEmpty()) {
        return;
    }

    ensureFileExtension(fileName, ".panda");

    reg->extractToFile(blobName, fileName);
    m_palette->updateICList(icListFile());
    m_palette->updateEmbeddedICList(scene);
    m_ui->statusBar->showMessage(i18n("IC extracted to %1", fileName), 4000);
}

void MainWindow::makeSelfContained()
{
    sentryBreadcrumb("ic", QStringLiteral("Make self-contained"));
    if (!m_currentTab) {
        return;
    }

    auto *scene = m_currentTab->scene();
    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        MessageDialog::warning(this, i18n("Please save the project first."), i18n("Error"));
        return;
    }

    // Collect unique file paths from all file-backed ICs
    QStringList uniqueFiles;
    for (auto *elm : scene->elements()) {
        if (elm->elementType() != ElementType::IC || elm->isEmbedded()) {
            continue;
        }
        const QString icFile = static_cast<IC *>(elm)->file();
        if (!icFile.isEmpty() && !uniqueFiles.contains(icFile)) {
            uniqueFiles.append(icFile);
        }
    }

    if (uniqueFiles.isEmpty()) {
        m_ui->statusBar->showMessage(i18n("No file-based ICs to embed."), 4000);
        return;
    }

    int totalEmbedded = 0;
    auto *reg = scene->icRegistry();

    for (const QString &icFile : std::as_const(uniqueFiles)) {
        const QString fullPath = QDir(contextDir).absoluteFilePath(icFile);
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly)) {
            MessageDialog::warning(this, i18n("Could not read IC file: %1", file.errorString()), i18n("Error"));
            break;
        }
        QByteArray fileBytes = file.readAll();
        file.close();

        QString blobName = resolveUniqueBlobName(QFileInfo(icFile).baseName(), scene);
        if (blobName.isEmpty()) {
            break; // User cancelled
        }

        totalEmbedded += reg->embedICsByFile(fullPath, fileBytes, blobName);
    }

    m_palette->updateEmbeddedICList(scene);
    m_ui->statusBar->showMessage(i18n("Embedded %1 IC(s). Circuit is now self-contained.", totalEmbedded), 4000);
}

