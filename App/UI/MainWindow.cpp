// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/MainWindow.h"

#include <algorithm>
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
#include <QLocale>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPixmapCache>
#include <QPushButton>
#include <QSaveFile>
#include <QShortcut>
#include <QTemporaryFile>

#ifdef Q_OS_MAC
#include <QSvgRenderer>
#endif

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/RecentFiles.h"
#include "App/IO/Serialization.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/ExportController.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/ICController.h"
#include "App/UI/LanguageManager.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/SceneUiBinder.h"
#include "App/UI/UpdateController.h"
#include "App/Versions.h"

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
    m_binder = new SceneUiBinder(m_ui.get(), m_palette, this, this);

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
    m_recentFiles = new RecentFiles(this);
    connect(this,          &MainWindow::addRecentFile,         m_recentFiles, &RecentFiles::addRecentFile);
    connect(m_recentFiles, &RecentFiles::recentFilesUpdated,   this,          &MainWindow::updateRecentFileActions);
    createRecentFileActions();
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
    // The scene-property shortcuts ( [ ] { } < > ) are owned by SceneUiBinder, which
    // re-targets them to the active tab's scene on each switch.
    auto *searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
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
    connect(m_ui->actionExportToArduino,       &QAction::triggered,       m_exportController,  &ExportController::exportArduinoDialog);
    connect(m_ui->actionExportToSystemVerilog, &QAction::triggered,       m_exportController,  &ExportController::exportSystemVerilogDialog);
    connect(m_ui->actionExportToImage,         &QAction::triggered,       m_exportController,  &ExportController::exportImageDialog);
    connect(m_ui->actionExportToPdf,           &QAction::triggered,       m_exportController,  &ExportController::exportPdfDialog);
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
    connect(m_ui->elementEditor, &ElementEditor::embedSubcircuitRequested, m_icController, &ICController::embedSelectedIC);
    connect(m_ui->elementEditor, &ElementEditor::extractToFileRequested, m_icController, &ICController::extractSelectedIC);
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
                QMessageBox::critical(nullptr, tr("Error!"), e.what());
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
    m_ui->tab->addTab(workspace, tr("New Project"));
    sentryBreadcrumb("ui", QStringLiteral("Tab opened"));

    qCDebug(zero) << "Selecting the newly created tab.";
    m_ui->tab->setCurrentIndex(m_ui->tab->count() - 1);

    qCDebug(zero) << "Finished #tabs: " << m_ui->tab->count() << ", current tab: " << m_tabIndex;
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
    Application::guardedSlot(this, [this] {
        close();
    });
}

void MainWindow::save(const QString &fileName)
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->save(fileName);
    m_palette->updateICList(icListFile());
    m_ui->statusBar->showMessage(tr("File saved successfully."), 4000);
}

void MainWindow::show()
{
    QMainWindow::show();

    qCDebug(zero) << "Checking for autosave file recovery.";
    loadAutosaveFiles();

    auto *updateController = new UpdateController(this);
    updateController->checkForUpdates();
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

int MainWindow::closeTabAnyway()
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("File not saved. Close tab anyway?"));
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

    const QString fileName = currentFile().fileName().isEmpty() ? tr("New Project") : currentFile().fileName();

    msgBox.setText(fileName + tr(" has been modified.\nDo you want to save your changes?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

void MainWindow::on_actionNew_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ui", QStringLiteral("New project"));
        createNewTab();
    });
}

void MainWindow::on_actionWires_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Wires: %1").arg(checked));
        if (m_currentTab) {
            m_currentTab->scene()->showWires(checked);
        }
    });
}

void MainWindow::on_actionRotateRight_triggered()
{
    Application::guardedSlot(this, [this] {
        if (m_currentTab) {
            m_currentTab->scene()->rotateRight();
        }
    });
}

void MainWindow::on_actionRotateLeft_triggered()
{
    Application::guardedSlot(this, [this] {
        if (m_currentTab) {
            m_currentTab->scene()->rotateLeft();
        }
    });
}

void MainWindow::loadPandaFile(const QString &fileName)
{
    const QFileInfo newFileInfo(fileName);

    for (int i = 0; i < m_ui->tab->count(); ++i) {
        if (auto *workspace = qobject_cast<WorkSpace *>(m_ui->tab->widget(i))) {
            if (workspace->fileInfo() == newFileInfo) {
                m_ui->tab->setCurrentIndex(i);
                return;
            }
        }
    }

    createNewTab();
    qCDebug(zero) << "Loading in editor.";
    try {
        m_currentTab->load(fileName);
    } catch (...) {
        // Drop the half-populated tab so we don't leak an empty workspace
        // into the tab bar. Clear the undo stack first so closeTab doesn't
        // prompt the user to save the partial load.
        m_currentTab->scene()->undoStack()->clear();
        closeTab(m_tabIndex);
        throw;
    }
    // Tighten the scene rect to the loaded items immediately so subsequent
    // interactions (selection, drag release) don't cause a viewport jump.
    m_currentTab->scene()->resizeScene();
    m_palette->updateICList(icListFile());
    m_palette->updateEmbeddedICList(m_currentTab->scene());
    m_ui->statusBar->showMessage(tr("File loaded successfully."), 4000);
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
    Application::guardedSlot(this, [this] {
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
        const QString fileName = FileDialogs::provider()->getOpenFileName(this, tr("Open File"), path, tr("Panda files (*.panda)"));

        if (fileName.isEmpty()) {
            return;
        }

        loadPandaFile(fileName);
    #endif
    });
}

void MainWindow::on_actionSave_triggered()
{
    Application::guardedSlot(this, [this] {
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
            fileName = FileDialogs::provider()->getSaveFileName(this, tr("Save File as ..."), QString(), tr("Panda files (*.panda)")).fileName;

            if (fileName.isEmpty()) {
                return;
            }

            ensureFileExtension(fileName, ".panda");
        }

        const int conflictTab = findTabWithFile(fileName);
        if (conflictTab != -1) {
            QMessageBox msgBox(QMessageBox::Warning,
                               tr("File Conflict"),
                               tr("The file \"%1\" is already open in another tab.").arg(QFileInfo(fileName).fileName()),
                               QMessageBox::Ok,
                               this);
            QPushButton *switchBtn = msgBox.addButton(tr("Switch to Tab"), QMessageBox::ActionRole);
            msgBox.exec();
            if (msgBox.clickedButton() == switchBtn) {
                m_ui->tab->setCurrentIndex(conflictTab);
            }
            return;
        }

        save(fileName);
    #endif
    });
}

void MainWindow::on_actionSaveAs_triggered()
{
    Application::guardedSlot(this, [this] {
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
        QString fileName = FileDialogs::provider()->getSaveFileName(this, tr("Save File as ..."), currentFile().absoluteFilePath(), tr("Panda files (*.panda)")).fileName;

        if (fileName.isEmpty()) {
            return;
        }

        ensureFileExtension(fileName, ".panda");

        const int conflictTab = findTabWithFile(fileName);
        if (conflictTab != -1) {
            QMessageBox msgBox(QMessageBox::Warning,
                               tr("File Conflict"),
                               tr("The file \"%1\" is already open in another tab.").arg(QFileInfo(fileName).fileName()),
                               QMessageBox::Ok,
                               this);
            QPushButton *switchBtn = msgBox.addButton(tr("Switch to Tab"), QMessageBox::ActionRole);
            msgBox.exec();
            if (msgBox.clickedButton() == switchBtn) {
                m_ui->tab->setCurrentIndex(conflictTab);
            }
            return;
        }

        save(fileName);
    #endif
    });
}

int MainWindow::findTabWithFile(const QString &fileName) const
{
    const QFileInfo newFileInfo(fileName);
    for (int i = 0; i < m_ui->tab->count(); ++i) {
        if (auto *workspace = qobject_cast<WorkSpace *>(m_ui->tab->widget(i))) {
            if (workspace != m_currentTab && workspace->fileInfo() == newFileInfo) {
                return i;
            }
        }
    }
    return -1;
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
               "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
                .arg(QApplication::applicationVersion()));
    });
}

void MainWindow::on_actionShortcuts_and_Tips_triggered()
{
    Application::guardedSlot(this, [this] {
        QMessageBox::information(this,
            tr("Shortcuts and Tips"),
            tr("<h1>Canvas Shortcuts</h1>"
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
            ));
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
                tr("Exit") + " " + QApplication::applicationName(),
                tr("Are you sure?"),
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
    const bool hasFile = m_currentTab && m_currentTab->fileInfo().isReadable();
    m_ui->pushButtonAddIC->setEnabled(hasFile);
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
    on_actionSave_triggered();
}

void MainWindow::showStatusMessage(const QString &message, int timeout)
{
    m_ui->statusBar->showMessage(message, timeout);
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
        text = fileInfo.exists() ? fileInfo.fileName() : tr("New Project");
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

    if (senderWs == m_currentTab) {
        refreshICButtonsEnabled();
    }
}

void MainWindow::on_actionSelectAll_triggered()
{
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ui", QStringLiteral("Select all"));
        if (!m_currentTab) {
            return;
        }

        m_currentTab->scene()->selectAll();
    });
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
                QMessageBox::critical(this, tr("Error"), e.what());

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
    // Called before switching away from a tab. SceneUiBinder tears down the chrome
    // wiring; the IC-open navigation connection is owned here (it drives tab/file
    // opening, not chrome).
    if (!m_currentTab) {
        return;
    }

    disconnect(m_currentTab->scene(), &Scene::icOpenRequested, this, nullptr);
    m_binder->unbind();
}

void MainWindow::connectTab()
{
    m_binder->bind(m_currentTab);

    // Tab-navigation wiring (not chrome): the file-based IC list, IC tool-button
    // state, and the scene's open-IC-in-tab / open-file requests.
    m_palette->updateICList(icListFile());

    // Hide management buttons for inline IC tabs (they use currentFile/currentDir which are empty)
    setICButtonsVisible(!m_currentTab->isInlineIC());
    refreshICButtonsEnabled();

    connect(m_currentTab->scene(), &Scene::icOpenRequested, this, [this](int elementId, const QString &blobName, const QString &filePath) {
        if (!blobName.isEmpty()) {
            if (m_currentTab) {
                openICInTab(blobName, elementId, m_currentTab->scene()->icRegistry()->blob(blobName));
            }
        } else if (!filePath.isEmpty()) {
            loadPandaFile(filePath);
        }
    });
}

WorkSpace *MainWindow::currentTab() const
{
    return m_currentTab;
}

ICPreviewPopup *MainWindow::icPreviewPopup() const
{
    return m_icPreviewPopup;
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
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("file", QStringLiteral("Reload file"));
        if (!currentFile().exists() || !m_currentTab) {
            return;
        }

        const QString file = currentFile().absoluteFilePath();

        closeTab(m_tabIndex);
        loadPandaFile(file);
    });
}

void MainWindow::on_actionGates_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
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
        if (!m_currentTab) {
            return;
        }

        m_currentTab->view()->zoomIn();
    });
}

void MainWindow::on_actionZoomOut_triggered() const
{
    Application::guardedSlot(this, [this] {
        if (!m_currentTab) {
            return;
        }

        m_currentTab->view()->zoomOut();
    });
}

void MainWindow::on_actionResetZoom_triggered() const
{
    Application::guardedSlot(this, [this] {
        if (!m_currentTab) {
            return;
        }

        m_currentTab->view()->resetZoom();
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
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("simulation", QStringLiteral("Simulation restart"));
        if (!m_currentTab) {
            return;
        }

        m_currentTab->simulation()->restart();
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
        if (!m_currentTab) {
            return;
        }

        sentryBreadcrumb("ui", QStringLiteral("Waveform dialog opened"));
        qCDebug(zero) << "BD fileName: " << m_currentTab->dolphinFileName();
        auto *bewavedDolphin = new BewavedDolphin(m_currentTab->scene(), true, this);
        bewavedDolphin->createWaveform(m_currentTab->dolphinFileName());
        bewavedDolphin->show();
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
        if (!m_currentTab) {
            return;
        }

        m_currentTab->scene()->flipHorizontally();
    });
}

void MainWindow::on_actionFlipVertically_triggered()
{
    Application::guardedSlot(this, [this] {
        if (!m_currentTab) {
            return;
        }

        m_currentTab->scene()->flipVertically();
    });
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
    Application::guardedSlot(this, [this] {
        sentryBreadcrumb("ui", QStringLiteral("Fullscreen toggled"));
        isFullScreen() ? showNormal() : showFullScreen();
    });
}

void MainWindow::on_actionMute_triggered(const bool checked)
{
    Application::guardedSlot(this, [this, checked] {
        sentryBreadcrumb("ui", QStringLiteral("Mute toggled"));
        if (!m_currentTab) {
            return;
        }

        m_currentTab->simulation()->setUserMuted(checked);
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
