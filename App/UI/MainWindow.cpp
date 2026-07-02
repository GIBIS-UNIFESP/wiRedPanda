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
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLocale>
#include <QLoggingCategory>
#include <QMap>
#include <QMessageBox>
#include <QPixmapCache>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
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
#include "App/UI/SceneUiBinder.h"
#include "App/UI/UpdateController.h"
#include "App/UI/WorkspaceManager.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"

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
    : QMainWindow(parent)
    , m_ui(std::make_unique<MainWindowUi>())
{
    qCDebug(zero) << "wiRedPanda Version = " APP_VERSION " OR " << AppVersion::current;
    m_ui->setupUi(this);
    qCDebug(zero) << "Settings fileName: " << Settings::fileName();

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

    // Periodic timer to refresh the simulation-time label and waveform in temporal mode.
    m_simTimeTimer.setInterval(100);
    connect(&m_simTimeTimer, &QTimer::timeout, this, &MainWindow::updateSimTimeLabel);
    m_simTimeTimer.start();

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
            QMessageBox::warning(this, tr("Error"), openFolderFailureText);
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
            tr("Open My Exercises Folder"),
            tr("Could not create or access a folder for custom exercises."),
            Settings::completedExercises(),
            [this](const QString &path) { startExercise(path); });
    });
}

void MainWindow::setupToursMenu()
{
    connect(m_ui->menuTours, &QMenu::aboutToShow, this, [this] {
        populateContentMenu(m_ui->menuTours, "Tours",
            tr("Open My Tours Folder"),
            tr("Could not create or access a folder for custom tours."),
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
    connect(m_ui->actionStepForward,           &QAction::triggered,       this,                &MainWindow::on_actionStepForward_triggered);
    connect(m_ui->actionStepBack,              &QAction::triggered,       this,                &MainWindow::on_actionStepBack_triggered);
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
    connect(m_ui->elementEditor, &ElementEditor::watchICInternalsRequested, this, &MainWindow::watchICInternals);
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
    connect(m_ui->comboSimMode,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_comboSimMode_currentIndexChanged);
    connect(m_ui->comboSimSpeed, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_comboSimSpeed_currentIndexChanged);
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
    // Sever the per-tab destroyed→cleanup hooks (dolphinForTab()) while members are still
    // alive: QWidget's destructor deletes child widgets (the tabs) BEFORE ~QObject severs
    // signal connections, so a WorkSpace::destroyed fired during teardown would run the
    // cleanup lambda against the already-destructed m_dolphins hash.
    for (auto it = m_dolphins.cbegin(); it != m_dolphins.cend(); ++it) {
        if (it.key()) {
            disconnect(it.key(), &QObject::destroyed, this, nullptr);
        }
    }

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
        tr("wiRedPanda %1\n\n"
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
            tr("<p>wiRedPanda is software developed by students of the Federal University of São Paulo"
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
    byLabel.insert(tr("Redo"), QKeySequence(QKeySequence::Redo).toString(QKeySequence::NativeText));
    byLabel.insert(tr("Undo"), QKeySequence(QKeySequence::Undo).toString(QKeySequence::NativeText));

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

    return tr("<h1>Keyboard Shortcuts</h1>"
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
              "</ul>")
        .arg(rows);
}

void MainWindow::on_actionShortcuts_and_Tips_triggered()
{
    Application::guardedSlot(this, [this] {
        QMessageBox::information(this, tr("Shortcuts and Tips"), shortcutsHelpHtml());
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
                tr("Exit") + " " + QApplication::applicationName(),
                tr("Are you sure?"),
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

    // The waveform dock shows the CURRENT tab's BewavedDolphin. Swap in the new tab's
    // instance (its grid document and analyzer watch list persist inside it); if this tab
    // never opened the tool, auto-hide the dock and remember why, so returning to a tab
    // that has one re-reveals it — but a dock the USER closed stays closed.
    if (m_dolphinDock) {
        BewavedDolphin *dolphin = newTab ? m_dolphins.value(newTab).data() : nullptr;
        if (dolphin) {
            if (m_dolphinDock->widget() != dolphin) {
                if (auto *old = m_dolphinDock->widget()) {
                    old->setParent(this); // keep the swapped-out instance alive, hidden
                    old->hide();
                }
                m_dolphinDock->setWidget(dolphin);
            }
            if (m_dolphinDockAutoHidden) {
                m_dolphinDock->show();
                m_dolphinDockAutoHidden = false;
            }
            // Tour/exercise "bwd" context always targets whichever instance the dock is
            // currently showing (see showDolphinDock()).
            m_bwd = dolphin;
        } else if (m_dolphinDock->isVisible()) {
            m_dolphinDock->hide();
            m_dolphinDockAutoHidden = true;
        }
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

    // Apply the current UI sim-mode/speed to the new tab's simulation (functional ⇒ 0,
    // temporal ⇒ the selected ns-per-tick window).
    if (m_ui->comboSimMode->currentIndex() == 1) {
        const SimTime nsPerTick = m_ui->comboSimSpeed->itemData(m_ui->comboSimSpeed->currentIndex()).toULongLong();
        newTab->simulation()->setTimePerTick(nsPerTick);
    } else {
        newTab->simulation()->setTimePerTick(0);
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
    setWindowTitle(tr("%1[*] — wiRedPanda %2")
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
            text = fileInfo.exists() ? fileInfo.fileName() : tr("New Project");
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

namespace {

/// Human-readable sim-time with the same unit ladder as the toolbar label: switch as soon
/// as the next unit reaches 1.0, so it reads "5.00 µs" rather than "5000 ns".
QString formatSimTime(const SimTime ns)
{
    if (ns >= 1'000'000'000) {
        return QString::number(static_cast<double>(ns) / 1'000'000'000.0, 'f', 2) + " s";
    }
    if (ns >= 1'000'000) {
        return QString::number(static_cast<double>(ns) / 1'000'000.0, 'f', 2) + " ms";
    }
    if (ns >= 1'000) {
        return QString::number(static_cast<double>(ns) / 1'000.0, 'f', 2) + QString::fromUtf8(" \xC2\xB5s");
    }
    return QString::number(ns) + " ns";
}

/// Display names for the status bar: the user's label when set, else the element type name.
QString stepElementNames(const QVector<GraphicElement *> &elements)
{
    QStringList names;
    for (auto *element : elements) {
        if (!element) {
            continue;
        }
        const QString label = element->label();
        names.append(label.isEmpty() ? ElementFactory::translatedName(element->elementType()) : label);
    }
    return names.join(QStringLiteral(", "));
}

} // namespace

void MainWindow::on_actionPlay_toggled(const bool checked)
{
    sentryBreadcrumb("simulation", QStringLiteral("Play toggled: %1").arg(checked));
    if (!currentTab()) {
        return;
    }

    auto *simulation = currentTab()->simulation();

    // The action is checkable; its toggled(bool) signal drives start/stop directly.
    checked ? simulation->start() : simulation->stop();

    // start() ends any step-debugger session (a "continue"): reflect that in the UI.
    if (checked) {
        m_ui->actionStepBack->setEnabled(false);
        currentTab()->scene()->setSteppedElements({});
    }
}

void MainWindow::on_actionStepForward_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("simulation", QStringLiteral("Step forward"));
        if (!currentTab()) {
            return;
        }

        // A debugger break: stepping while running pauses first, then takes the step.
        if (m_ui->actionPlay->isChecked()) {
            m_ui->actionPlay->setChecked(false); // toggled() drives simulation->stop()
        }

        reportStepResult(currentTab()->simulation()->stepPropagation());
    });
}

void MainWindow::on_actionStepBack_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("simulation", QStringLiteral("Step back"));
        if (!currentTab()) {
            return;
        }

        reportStepResult(currentTab()->simulation()->stepBack());
    });
}

void MainWindow::reportStepResult(const Simulation::StepResult &result)
{
    if (!currentTab()) {
        return;
    }
    auto *simulation = currentTab()->simulation();
    auto *scene = currentTab()->scene();

    switch (result.status) {
    case Simulation::StepStatus::Stepped: {
        scene->setSteppedElements(result.changed);
        QString message = tr("Stepped: %1").arg(stepElementNames(result.changed));
        if (simulation->timePerTick() > 0) {
            message += tr(" — t = %1").arg(formatSimTime(simulation->currentTime()));
        }
        showStatusMessage(message, 5000);
        break;
    }
    case Simulation::StepStatus::Settled:
        scene->setSteppedElements({});
        showStatusMessage(tr("Circuit settled — toggle an input to continue stepping."), 5000);
        break;
    case Simulation::StepStatus::NotReady:
        showStatusMessage(tr("Nothing to step — the circuit has no simulation yet."), 5000);
        break;
    }

    m_ui->actionStepBack->setEnabled(simulation->canStepBack());
    updateSimTimeLabel();
}

void MainWindow::on_actionRestart_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("simulation", QStringLiteral("Simulation restart"));
        if (!currentTab()) {
            return;
        }

        currentTab()->simulation()->restart();

        // restart() wipes the step-debugger session along with the timeline.
        m_ui->actionStepBack->setEnabled(false);
        currentTab()->scene()->setSteppedElements({});
    });
}

void MainWindow::on_comboSimMode_currentIndexChanged(int index)
{
    Application::guardedSlot(this, [this, index] {
        if (!currentTab()) {
            return;
        }

        const bool temporal = (index == 1);
        m_ui->actionSimSpeed->setVisible(temporal);
        m_ui->actionSimTime->setVisible(temporal);

        if (temporal) {
            // Temporal: apply the current speed (sets the ns-per-tick window) and adapt zoom.
            on_comboSimSpeed_currentIndexChanged(m_ui->comboSimSpeed->currentIndex());
            m_ui->labelSimTime->setText(tr("0 ns"));
        } else {
            // Functional (zero-delay): every tick is a full settle at the current instant.
            currentTab()->simulation()->setTimePerTick(0);
        }

        // Re-settle the circuit under the new mode. Without this, the mode change only swaps the tick
        // window: a feedback circuit that canonicalized to Unknown in functional mode would stay stuck
        // there when switched to temporal (NOT(Unknown) is a fixed point). initialize() resets every
        // element's outputs to Inactive and forces a fresh seed, so e.g. a ring oscillator starts
        // oscillating the moment Temporal mode is selected.
        auto *simulation = currentTab()->simulation();
        if (!simulation->initialize()) {
            simulation->restart();
        }
    });
}

void MainWindow::on_comboSimSpeed_currentIndexChanged(int index)
{
    Application::guardedSlot(this, [this, index] {
        if (!currentTab()) {
            return;
        }

        // The Live Analyzer adapts its zoom itself via Simulation::timePerTickChanged.
        const SimTime nsPerTick = m_ui->comboSimSpeed->itemData(index).toULongLong();
        currentTab()->simulation()->setTimePerTick(nsPerTick);
    });
}

void MainWindow::updateSimTimeLabel()
{
    Application::guardedSlot(this, [this] {
        if (!currentTab() || !m_ui->actionSimTime->isVisible()) {
            return;
        }

        m_ui->labelSimTime->setText(formatSimTime(currentTab()->simulation()->currentTime()));
    });
}

BewavedDolphin *MainWindow::dolphinForTab(WorkSpace *tab)
{
    if (!tab) {
        return nullptr;
    }
    auto &entry = m_dolphins[tab];
    if (!entry) {
        entry = new BewavedDolphin(tab->scene(), true, this);
        try {
            entry->createWaveform(tab->dolphinFileName());
        } catch (const Pandaception &) {
            // The stimulus grid needs top-level inputs/outputs; a scene without them (e.g.
            // only an IC being probed) must still get the waveform tool — the Live Analyzer
            // works regardless, so host in analyzer-only mode instead of failing.
            entry->disableStimulusEditor();
            m_ui->statusBar->showMessage(tr("No inputs/outputs for the stimulus editor — opening the Live Analyzer."), 5000);
        }

        // The instance is bound to this tab's scene/simulation; when the tab dies, its
        // waveform tool dies with it (and vacates the dock if it was showing there).
        // ~MainWindow severs this hook explicitly — see the destructor for why.
        connect(tab, &QObject::destroyed, this, [this, tab] {
            BewavedDolphin *dolphin = m_dolphins.take(tab);
            if (dolphin) {
                if (m_dolphinDock && m_dolphinDock->widget() == dolphin) {
                    m_dolphinDock->setWidget(nullptr);
                    m_dolphinDock->hide();
                }
                dolphin->deleteLater();
            }
        });
    }
    return entry;
}

void MainWindow::showDolphinDock(WorkSpace *tab)
{
    if (!tab) {
        return;
    }
    if (!m_dolphinDock) {
        m_dolphinDock = new QDockWidget(tr("Waveform"), this);
        m_dolphinDock->setObjectName("waveformDock");
        addDockWidget(Qt::BottomDockWidgetArea, m_dolphinDock);
    }

    auto *dolphin = dolphinForTab(tab);
    if (m_dolphinDock->widget() != dolphin) {
        if (auto *old = m_dolphinDock->widget()) {
            old->setParent(this); // keep the swapped-out instance alive, hidden
            old->hide();
        }
        m_dolphinDock->setWidget(dolphin);
    }
    m_dolphinDock->show();
    m_dolphinDockAutoHidden = false;
    // Tour/exercise steps with a "bwd" context target whichever instance is currently
    // visible in the dock (QPointer, so it auto-nulls if this tab's instance is later freed).
    m_bwd = dolphin;
    dolphin->show(); // BewavedDolphin::show() also re-applies the table zoom
}

void MainWindow::watchICInternals(IC *ic)
{
    Application::guardedSlot(this, [this, ic] {
        if (!currentTab() || !ic) {
            return;
        }

        // Route to the tab's waveform tool: reveal the dock on its Live Analyzer page and
        // let the panel do the recursive watching.
        showDolphinDock(currentTab());
        if (auto *dolphin = m_dolphins.value(currentTab()).data()) {
            dolphin->watchICInternals(ic);
        }
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
        if (!currentTab()) {
            return;
        }

        sentryBreadcrumb("ui", QStringLiteral("Waveform dock opened"));
        qCDebug(zero) << "BD fileName: " << currentTab()->dolphinFileName();
        showDolphinDock(currentTab());
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
        m_ui->actionMute->setText(checked ? tr("Unmute") : tr("Mute"));
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
