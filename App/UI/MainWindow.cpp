// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/MainWindow.h"

#ifdef Q_OS_WASM
#include <emscripten/emscripten.h>
#endif

#include <QActionGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLocale>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPdfWriter>
#include <QPixmapCache>
#include <QPrinter>
#include <QResource>
#include <QSaveFile>
#include <QShortcut>
#include <QTemporaryFile>
#include <QTranslator>

#ifdef Q_OS_MAC
#include <QSvgRenderer>
#endif

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/Core/Common.h"
#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/IO/RecentFiles.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/MainWindowUI.h"

#ifdef Q_OS_MAC
void ensureSvgUsage() {
    QSvgRenderer dummy; // for macdeployqt to add libqsvg.dylib
}
#endif

MainWindow::MainWindow(const QString &fileName, QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<MainWindowUi>())
{
    qCDebug(zero) << "wiRedPanda Version = " APP_VERSION " OR " << GlobalProperties::version;
    m_ui->setupUi(this);

    qCDebug(zero) << "Settings fileName: " << Settings::fileName();

    // --- Language / translation setup ---
    // Get language from settings, or auto-detect from system if not set
    QString language = Settings::value("language").toString();
    if (language.isEmpty()) {
        // Auto-detect system language
        QLocale systemLocale = QLocale::system();
        QString systemLanguage = systemLocale.name(); // e.g., "en_US", "pt_BR"
        QString baseLanguage = systemLanguage.split('_').first(); // e.g., "en", "pt"

        qCDebug(zero) << "Auto-detected system locale:" << systemLanguage;

        // Check if we have translation for this language
        const auto availableLanguages = getAvailableLanguages();

        // First try the full locale (e.g., "pt_BR")
        if (availableLanguages.contains(systemLanguage)) {
            language = systemLanguage;
        }
        // Then try the base language (e.g., "pt")
        else if (availableLanguages.contains(baseLanguage)) {
            language = baseLanguage;
        }
        // Fall back to English
        else {
            qCDebug(zero) << "No translation available for" << systemLanguage << "or" << baseLanguage << ", falling back to English";
            language = "en";
        }

        qCDebug(zero) << "Selected language:" << language;
    }

    loadTranslation(language);
    populateLanguageMenu();

    connect(m_ui->tab, &QTabWidget::currentChanged,    this, &MainWindow::tabChanged);
    connect(m_ui->tab, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    qCDebug(zero) << "Restoring geometry and setting zoom controls.";
    restoreGeometry(Settings::value("MainWindow/geometry").toByteArray());
    restoreState(Settings::value("MainWindow/windowState").toByteArray());
    m_ui->splitter->restoreGeometry(Settings::value("MainWindow/splitter/geometry").toByteArray());
    m_ui->splitter->restoreState(Settings::value("MainWindow/splitter/state").toByteArray());

    qCDebug(zero) << "Preparing theme and UI modes.";
    auto *themeGroup = new QActionGroup(this);
    const auto actions = m_ui->menuTheme->actions();

    for (auto *action : actions) {
        themeGroup->addAction(action);
    }

    themeGroup->setExclusive(true);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &MainWindow::updateTheme);
    updateTheme();
    setFastMode(Settings::value("fastMode").toBool());
    m_ui->actionLabelsUnderIcons->setChecked(Settings::value("labelsUnderIcons").toBool());
    m_ui->mainToolBar->setToolButtonStyle(Settings::value("labelsUnderIcons").toBool() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);

    qCDebug(zero) << "Setting left side menus.";
    populateLeftMenu();

    // --- Left-panel tab icon setup ---
    // Lookup by object name rather than positional index so reordering tabs doesn't break icons.
    // Set tab icons using object name-based lookup for robustness
    int ioTabIndex = getTabIndex("io");
    int gatesTabIndex = getTabIndex("gates");
    int combinationalTabIndex = getTabIndex("combinational");
    int memoryTabIndex = getTabIndex("memory");
    int icTabIndex = getTabIndex("ic");
    int miscTabIndex = getTabIndex("misc");
    int searchTabIndex = getTabIndex("search");

    if (ioTabIndex != -1) m_ui->tabElements->setTabIcon(ioTabIndex, QIcon(":/Components/Input/buttonOff.svg"));
    if (gatesTabIndex != -1) m_ui->tabElements->setTabIcon(gatesTabIndex, QIcon(":/Components/Logic/xor.svg"));
    if (combinationalTabIndex != -1) m_ui->tabElements->setTabIcon(combinationalTabIndex, QIcon(":/Components/Logic/truthtable-rotated.svg"));
    if (memoryTabIndex != -1) m_ui->tabElements->setTabIcon(memoryTabIndex, QIcon(DFlipFlop::pixmapPath()));
    if (icTabIndex != -1) m_ui->tabElements->setTabIcon(icTabIndex, QIcon(":/Components/Logic/ic-panda.svg"));
    if (miscTabIndex != -1) m_ui->tabElements->setTabIcon(miscTabIndex, QIcon(":/Components/Misc/text.png"));
    // Search tab is a virtual tab that only becomes enabled when the user types in the search box.
    if (searchTabIndex != -1) m_ui->tabElements->setTabEnabled(searchTabIndex, false);

    qCDebug(zero) << "Loading recent file list.";
    m_recentFiles = new RecentFiles(this);
    connect(this, &MainWindow::addRecentFile, m_recentFiles, &RecentFiles::addRecentFile);
    createRecentFileActions();
    connect(m_recentFiles, &RecentFiles::recentFilesUpdated, this, &MainWindow::updateRecentFileActions);

    qCDebug(zero) << "Checking playing simulation.";
    // Start simulation running by default so the circuit is live on open.
    m_ui->actionPlay->setChecked(true);

    qCDebug(zero) << "Window title.";
    setWindowTitle("wiRedPanda " APP_VERSION);

    qCDebug(zero) << "Building a new tab.";
    createNewTab();

    qCDebug(zero) << "Opening file if not empty.";
    if (!fileName.isEmpty()) {
        loadPandaFile(fileName);
    }

    // Arduino export is experimentally disabled until the code generator is
    // complete enough for general use.
    qCDebug(zero) << "Disabling Arduino export.";
    m_ui->actionExportToArduino->setEnabled(false);

    // 100 000 KB — large circuits with many IC pixmaps benefit from generous caching;
    // the default Qt limit of 10 000 KB causes frequent cache misses on complex designs.
    QPixmapCache::setCacheLimit(100000);

    qCDebug(zero) << "Adding examples to menu";

    const QString appDir = QCoreApplication::applicationDirPath();

    const QStringList searchPaths = {
        appDir + "/Examples",                       // Windows / dev builds
#ifdef Q_OS_MACOS
        appDir + "/../Resources/Examples",          // macOS app bundle
#endif
#ifdef Q_OS_LINUX
        qEnvironmentVariable("APPDIR") + "/usr/share/wiredpanda/Examples",  // AppImage
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
            auto *action = new QAction(entry);

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

    // --- Scene-level keyboard shortcuts ---
    // These are not in the menu bar, so they live here as QShortcut objects.
    // [ / ] cycle a selected element's primary property (e.g. input size).
    // { / } cycle a secondary property; < / > morph through element variants.
    auto *searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    auto *prevMainPropShortcut = new QShortcut(QKeySequence("["), this);
    auto *nextMainPropShortcut = new QShortcut(QKeySequence("]"), this);
    auto *prevSecndPropShortcut = new QShortcut(QKeySequence("{"), this);
    auto *nextSecndPropShortcut = new QShortcut(QKeySequence("}"), this);
    auto *changePrevElmShortcut = new QShortcut(QKeySequence("<"), this);
    auto *changeNextElmShortcut = new QShortcut(QKeySequence(">"), this);

    connect(searchShortcut,               &QShortcut::activated,      m_ui->lineEditSearch,  qOverload<>(&QWidget::setFocus));
    connect(prevMainPropShortcut,         &QShortcut::activated,      m_currentTab->scene(), &Scene::prevMainPropShortcut);
    connect(nextMainPropShortcut,         &QShortcut::activated,      m_currentTab->scene(), &Scene::nextMainPropShortcut);
    connect(prevSecndPropShortcut,        &QShortcut::activated,      m_currentTab->scene(), &Scene::prevSecndPropShortcut);
    connect(nextSecndPropShortcut,        &QShortcut::activated,      m_currentTab->scene(), &Scene::nextSecndPropShortcut);
    connect(changePrevElmShortcut,        &QShortcut::activated,      m_currentTab->scene(), &Scene::prevElm);
    connect(changeNextElmShortcut,        &QShortcut::activated,      m_currentTab->scene(), &Scene::nextElm);

    qCDebug(zero) << "Setting connections";
    connect(m_ui->actionAbout,            &QAction::triggered,        this,                &MainWindow::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,          &QAction::triggered,        this,                &MainWindow::on_actionAboutQt_triggered);
    connect(m_ui->actionAboutThisVersion, &QAction::triggered,        this,                &MainWindow::aboutThisVersion);
    connect(m_ui->actionReportTranslationError, &QAction::triggered,  this,                &MainWindow::on_actionReportTranslationError_triggered);
    connect(m_ui->actionChangeTrigger,    &QAction::triggered,        m_ui->elementEditor, &ElementEditor::changeTriggerAction);
    connect(m_ui->actionDarkTheme,        &QAction::triggered,        this,                &MainWindow::on_actionDarkTheme_triggered);
    connect(m_ui->actionExit,             &QAction::triggered,        this,                &MainWindow::on_actionExit_triggered);
    connect(m_ui->actionExportToArduino,  &QAction::triggered,        this,                &MainWindow::on_actionExportToArduino_triggered);
    connect(m_ui->actionExportToImage,    &QAction::triggered,        this,                &MainWindow::on_actionExportToImage_triggered);
    connect(m_ui->actionExportToPdf,      &QAction::triggered,        this,                &MainWindow::on_actionExportToPdf_triggered);
    connect(m_ui->actionFastMode,         &QAction::triggered,        this,                &MainWindow::on_actionFastMode_triggered);
    connect(m_ui->actionFlipHorizontally, &QAction::triggered,        this,                &MainWindow::on_actionFlipHorizontally_triggered);
    connect(m_ui->actionFlipVertically,   &QAction::triggered,        this,                &MainWindow::on_actionFlipVertically_triggered);
    connect(m_ui->actionFullscreen,       &QAction::triggered,        this,                &MainWindow::on_actionFullscreen_triggered);
    connect(m_ui->actionGates,            &QAction::triggered,        this,                &MainWindow::on_actionGates_triggered);
    connect(m_ui->actionLabelsUnderIcons, &QAction::triggered,        this,                &MainWindow::on_actionLabelsUnderIcons_triggered);
    connect(m_ui->actionLightTheme,       &QAction::triggered,        this,                &MainWindow::on_actionLightTheme_triggered);
    connect(m_ui->actionMute,             &QAction::triggered,        this,                &MainWindow::on_actionMute_triggered);
    connect(m_ui->actionNew,              &QAction::triggered,        this,                &MainWindow::on_actionNew_triggered);
    connect(m_ui->actionOpen,             &QAction::triggered,        this,                &MainWindow::on_actionOpen_triggered);
    connect(m_ui->actionPlay,             &QAction::toggled,          this,                &MainWindow::on_actionPlay_toggled);
    connect(m_ui->actionReloadFile,       &QAction::triggered,        this,                &MainWindow::on_actionReloadFile_triggered);
    connect(m_ui->actionRename,           &QAction::triggered,        m_ui->elementEditor, &ElementEditor::renameAction);
    connect(m_ui->actionResetZoom,        &QAction::triggered,        this,                &MainWindow::on_actionResetZoom_triggered);
    connect(m_ui->actionRestart,          &QAction::triggered,        this,                &MainWindow::on_actionRestart_triggered);
    connect(m_ui->actionRotateLeft,       &QAction::triggered,        this,                &MainWindow::on_actionRotateLeft_triggered);
    connect(m_ui->actionRotateRight,      &QAction::triggered,        this,                &MainWindow::on_actionRotateRight_triggered);
    connect(m_ui->actionSave,             &QAction::triggered,        this,                &MainWindow::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,           &QAction::triggered,        this,                &MainWindow::on_actionSaveAs_triggered);
    connect(m_ui->actionSelectAll,        &QAction::triggered,        this,                &MainWindow::on_actionSelectAll_triggered);
    connect(m_ui->actionShortcutsAndTips, &QAction::triggered,        this,                &MainWindow::on_actionShortcuts_and_Tips_triggered);
    connect(m_ui->actionWaveform,         &QAction::triggered,        this,                &MainWindow::on_actionWaveform_triggered);
    connect(m_ui->actionWires,            &QAction::triggered,        this,                &MainWindow::on_actionWires_triggered);
    connect(m_ui->actionZoomIn,           &QAction::triggered,        this,                &MainWindow::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,          &QAction::triggered,        this,                &MainWindow::on_actionZoomOut_triggered);
    connect(m_ui->lineEditSearch,         &QLineEdit::returnPressed,  this,                &MainWindow::on_lineEditSearch_returnPressed);
    connect(m_ui->lineEditSearch,         &QLineEdit::textChanged,    this,                &MainWindow::on_lineEditSearch_textChanged);
    connect(m_ui->pushButtonAddIC,        &QPushButton::clicked,      this,                &MainWindow::on_pushButtonAddIC_clicked);
    connect(m_ui->pushButtonRemoveIC,     &QPushButton::clicked,      this,                &MainWindow::on_pushButtonRemoveIC_clicked);
    connect(m_ui->pushButtonRemoveIC,     &TrashButton::removeICFile, this,                &MainWindow::removeICFile);
}

MainWindow::~MainWindow()
{
}

void MainWindow::loadAutosaveFiles()
{
    QStringList autosaves(Settings::value("autosaveFile").toStringList());

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
            QMessageBox::critical(nullptr, tr("Error!"), e.what());
            qCDebug(zero) << "Removing autosave file that is corrupted.";
            it = autosaves.erase(it);
            continue;
        }

        // Mark the newly loaded tab so it knows it came from an autosave,
        // causing it to prompt for a real save path on the next Ctrl+S.
        m_currentTab->setAutosaveFile();

        ++it;
    }

    Settings::setValue("autosaveFile", autosaves);
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

    qCDebug(zero) << "Selecting the newly created tab.";
    m_ui->tab->setCurrentIndex(m_ui->tab->count() - 1);

    qCDebug(zero) << "Finished #tabs: " << m_ui->tab->count() << ", current tab: " << m_tabIndex;
}

void MainWindow::removeUndoRedoMenu()
{
    if (!m_currentTab) {
        return;
    }

    // Undo and Redo are the first two actions in menuEdit (inserted by
    // addUndoRedoMenu).  Removing index 0 twice pops them both; after the
    // first removal the previous index 1 becomes the new index 0.
    m_ui->menuEdit->removeAction(m_ui->menuEdit->actions().constFirst());
    m_ui->menuEdit->removeAction(m_ui->menuEdit->actions().constFirst());
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
    updateICList();
    m_ui->statusBar->showMessage(tr("File saved successfully."), 4000);
}

void MainWindow::show()
{
    QMainWindow::show();

    // Recovery is deferred to show() so that the window is fully visible
    // before any blocking message boxes are presented to the user.
    if (!Settings::contains("hideV4Warning")) {
        aboutThisVersion();
    }

    qCDebug(zero) << "Checking for autosave file recovery.";
    loadAutosaveFiles();
}

void MainWindow::aboutThisVersion()
{
    qCDebug(zero) << "'hideV4Warning' message box.";

    auto *checkBox = new QCheckBox(tr("Don't show this again."));

    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.setWindowTitle("wiRedPanda " APP_VERSION);
    msgBox.setText(
        tr("wiRedPanda version >= 4.0 is not 100% compatible with previous versions.\n"
           "To open old version projects containing ICs (or boxes), skins, and/or "
           "beWavedDolphin simulations, their files must be moved to the same directory "
           "as the main project file.\n"
           "wiRedPanda %1 will automatically list all other .panda files located "
           "in the same directory of the current project as ICs in the editor tab.\n"
           "You have to save new projects before accessing ICs and skins, or running "
           "beWavedDolphin simulations.").arg(APP_VERSION));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setCheckBox(checkBox);

#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(checkBox, &QCheckBox::stateChanged, this, [](int state) {
#else
    connect(checkBox, &QCheckBox::checkStateChanged, this, [](int state) {
#endif
        if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
            Settings::setValue("hideV4Warning", "true");
        } else {
            Settings::remove("hideV4Warning");
        }
    });

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

    const QString fileName = m_currentFile.fileName().isEmpty() ? tr("New Project") : m_currentFile.fileName();

    msgBox.setText(fileName + tr(" has been modified.\nDo you want to save your changes?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

void MainWindow::on_actionNew_triggered()
{
    createNewTab();
}

void MainWindow::on_actionWires_triggered(const bool checked)
{
    if (m_currentTab) {
        m_currentTab->scene()->showWires(checked);
    }
}

void MainWindow::on_actionRotateRight_triggered()
{
    if (m_currentTab) {
        m_currentTab->scene()->rotateRight();
    }
}

void MainWindow::on_actionRotateLeft_triggered()
{
    if (m_currentTab) {
        m_currentTab->scene()->rotateLeft();
    }
}

void MainWindow::loadPandaFile(const QString &fileName)
{
    createNewTab();
    qCDebug(zero) << "Loading in editor.";
    m_currentTab->load(fileName);
    updateICList();
    m_ui->statusBar->showMessage(tr("File loaded successfully."), 4000);
}

void MainWindow::on_actionOpen_triggered()
{
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
    const QString path = m_currentFile.exists() ? "" : "./examples";
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), path, tr("Panda files (*.panda)"));

    if (fileName.isEmpty()) {
        return;
    }

    loadPandaFile(fileName);
#endif
}

void MainWindow::on_actionSave_triggered()
{
    if (!m_currentTab) {
        return;
    }

    // TODO: if current file is autosave ask for filename

    // If the project has never been saved, fall through to a Save-As dialog.
    QString fileName = m_currentFile.absoluteFilePath();

    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(this, tr("Save File as ..."), "", tr("Panda files (*.panda)"));

        if (fileName.isEmpty()) {
            return;
        }

        if (!fileName.endsWith(".panda")) {
            fileName.append(".panda");
        }
    }

    save(fileName);
}

void MainWindow::on_actionSaveAs_triggered()
{
    if (!m_currentTab) {
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File as ..."), m_currentFile.absoluteFilePath(), tr("Panda files (*.panda)"));

    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".panda")) {
        fileName.append(".panda");
    }

    // IC sub-circuit files are stored alongside the main .panda file.
    // Copy them to the new location so the saved copy is self-contained.
    IC::copyFiles(QFileInfo(m_currentFile), QFileInfo(fileName));

    // Open the new file in a fresh tab rather than just saving in-place so
    // the tab title and m_currentFile update to reflect the new path.
    loadPandaFile(fileName);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(
        this,
        "wiRedPanda",
        tr("<p>wiRedPanda is a software developed by the students of the Federal University of São Paulo."
           " This project was created in order to help students learn about logic circuits.</p>"
           "<p>Software version: %1</p>"
           "<p><strong>Creators:</strong></p>"
           "<ul>"
           "<li> Davi Morales </li>"
           "<li> Lucas Lellis </li>"
           "<li> Rodrigo Torres </li>"
           "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
           "</ul>"
           "<p> wiRedPanda is currently maintained by Prof. Fábio Cappabianco, Ph.D., João Pedro M. Oliveira, Matheus R. Esteves and Maycon A. Santana.</p>"
           "<p> Please file a report at our GitHub page if bugs are found or if you wish for a new functionality to be implemented.</p>"
           "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
            .arg(QApplication::applicationVersion()));
}

void MainWindow::on_actionShortcuts_and_Tips_triggered()
{
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
           "<p>Double click in a wire to create a node</p>"
        ));
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
    // Close from last to first so that tab indices stay stable during iteration;
    // removing the last tab never shifts the indices of earlier tabs.
    while (m_ui->tab->count() != 0) {
        if (!closeTab(m_ui->tab->count() - 1)) {
            return false; // user cancelled — abort the close sequence
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
                tr("Exit ") + QApplication::applicationName(),
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
    Settings::setValue("MainWindow/geometry", saveGeometry());
    Settings::setValue("MainWindow/windowState", saveState());
    Settings::setValue("MainWindow/splitter/geometry", m_ui->splitter->saveGeometry());
    Settings::setValue("MainWindow/splitter/state", m_ui->splitter->saveState());
}

bool MainWindow::hasModifiedFiles()
{
    const QStringList autosaves = Settings::value("autosaveFile").toStringList();

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

void MainWindow::setCurrentFile(const QFileInfo &fileInfo)
{
    if (!m_currentTab) {
        return;
    }

    m_currentFile = fileInfo;

    QString text = fileInfo.exists() ? fileInfo.fileName() : tr("New Project");

    // Append an asterisk to the tab title to indicate unsaved changes,
    // following the common editor convention.
    if (m_currentTab) {
        auto *scene = m_currentTab->scene();
        if (scene) {
            auto *undoStack = scene->undoStack();
            if (undoStack && !undoStack->isClean()) {
                text += "*";
            }
        }
    }

    m_ui->tab->setTabText(m_tabIndex, text);
    // Full path in the tooltip so users can distinguish files with the same name.
    m_ui->tab->setTabToolTip(m_tabIndex, fileInfo.absoluteFilePath());

    qCDebug(zero) << "Adding file to recent files.";
    emit addRecentFile(fileInfo.absoluteFilePath());
}

void MainWindow::on_actionSelectAll_triggered()
{
    m_currentTab->scene()->selectAll();
}

void MainWindow::updateICList()
{
    // Remove the expanding spacer before inserting items so new labels don't
    // push it out of place; it's re-added at the bottom after all labels.
    m_ui->scrollAreaWidgetContents_IC->layout()->removeItem(m_ui->verticalSpacer_IC);

    // Clear all existing IC labels from both the IC panel and the search panel.
    const auto items = m_ui->scrollAreaWidgetContents_IC->findChildren<ElementLabel *>();

    for (auto *item : items) {
        item->deleteLater();
    }

    const auto items2 = m_ui->scrollAreaWidgetContents_Search->findChildren<ElementLabel *>();

    for (auto *item : items2) {
        if (item->elementType() == ElementType::IC) {
            item->deleteLater();
        }
    }

    if (m_currentFile.exists()) {
        qCDebug(zero) << "Show files.";
        QDir directory(m_currentFile.absoluteDir());
        // Enumerate all .panda files in the same directory — they are candidate ICs.
        QStringList files = directory.entryList({"*.panda", "*.PANDA"}, QDir::Files);
        // Exclude the project file itself from the IC list.
        files.removeAll(m_currentFile.fileName());

        // Skip hidden files (names starting with '.') — these are typically
        // autosave/temporary files, not user-created ICs.
        for (int i = static_cast<int>(files.size()) - 1; i >= 0; --i) {
            if (files.at(i).at(0) == '.') {
                files.removeAt(i);
            }
        }

        qCDebug(zero) << "Files: " << files.join(", ");
        for (const QString &file : std::as_const(files)) {
            QPixmap pixmap(":/Components/Logic/ic-panda.svg");

            // Each IC needs two label instances: one for the dedicated IC tab
            // and another mirrored into the search panel.
            auto *item = new ElementLabel(pixmap, ElementType::IC, file, this);
            m_ui->scrollAreaWidgetContents_IC->layout()->addWidget(item);

            auto *item2 = new ElementLabel(pixmap, ElementType::IC, file, this);
            m_ui->scrollAreaWidgetContents_Search->layout()->addWidget(item2);
        }
    }

    m_ui->scrollAreaWidgetContents_IC->layout()->addItem(m_ui->verticalSpacer_IC);
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

    qCDebug(zero) << "Disconnecting zoom from UI.";
    disconnect(m_currentTab->view(), &GraphicsView::zoomChanged, this, &MainWindow::zoomChanged);

    qCDebug(zero) << "Removing undo and redo actions from UI menu.";
    removeUndoRedoMenu();

    disconnect(m_ui->actionClearSelection, &QAction::triggered,         m_currentTab->scene(), &Scene::clearSelection);
    disconnect(m_ui->actionCopy,           &QAction::triggered,         m_currentTab->scene(), &Scene::copyAction);
    disconnect(m_ui->actionCut,            &QAction::triggered,         m_currentTab->scene(), &Scene::cutAction);
    disconnect(m_ui->actionDelete,         &QAction::triggered,         m_currentTab->scene(), &Scene::deleteAction);
    disconnect(m_ui->actionPaste,          &QAction::triggered,         m_currentTab->scene(), &Scene::pasteAction);
    disconnect(m_ui->elementEditor,        &ElementEditor::sendCommand, m_currentTab->scene(), &Scene::receiveCommand);
}

void MainWindow::connectTab()
{
    qCDebug(zero) << "Connecting undo and redo functions to UI menu.";
    addUndoRedoMenu();

    qCDebug(zero) << "Setting Panda file info.";
    m_currentFile = m_currentTab->fileInfo();

    updateICList();

    qCDebug(zero) << "Connecting current tab to element editor menu in UI.";
    m_ui->elementEditor->setScene(m_currentTab->scene());

    connect(m_currentTab->view(),       &GraphicsView::zoomChanged, this,                  &MainWindow::zoomChanged);
    connect(m_ui->actionClearSelection, &QAction::triggered,        m_currentTab->scene(), &Scene::clearSelection);
    connect(m_ui->actionCopy,           &QAction::triggered,        m_currentTab->scene(), &Scene::copyAction);
    connect(m_ui->actionCut,            &QAction::triggered,        m_currentTab->scene(), &Scene::cutAction);
    connect(m_ui->actionDelete,         &QAction::triggered,        m_currentTab->scene(), &Scene::deleteAction);
    connect(m_ui->actionPaste,          &QAction::triggered,        m_currentTab->scene(), &Scene::pasteAction);

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

    // Keep the global ID counter ahead of the highest ID already used in this
    // scene so that new elements receive unique IDs.
    ElementFactory::setLastId(m_currentTab->lastId());
}

WorkSpace *MainWindow::currentTab() const
{
    return m_currentTab;
}

void MainWindow::tabChanged(const int newTabIndex)
{
    disconnectTab(); // disconnect previously selected tab
    m_tabIndex = newTabIndex;
    // Hide the editor panel during the transition; connectTab() will restore it
    // once the new scene's selection is known.
    m_ui->elementEditor->hide();

    if (newTabIndex == -1) {
        // All tabs were closed; reset state.
        m_currentTab = nullptr;
        return;
    }

    m_currentTab = qobject_cast<WorkSpace *>(m_ui->tab->currentWidget());
    qCDebug(zero) << "Selecting tab: " << newTabIndex;
    connectTab();
    qCDebug(zero) << "New tab selected. Dolphin fileName: " << m_currentTab->dolphinFileName();
}

void MainWindow::on_lineEditSearch_textChanged(const QString &text)
{
    // Remove spacer before modifying visible items to avoid layout artefacts;
    // it's re-added at the end.
    m_ui->scrollAreaWidgetContents_Search->layout()->removeItem(m_ui->verticalSpacer_Search);

    const int searchTabIndex = getTabIndex("search");

    if (text.isEmpty()) {
        // Restore the normal tab bar and return to the tab the user was on before
        // they started typing.
        m_ui->tabElements->tabBar()->show();
        m_ui->tabElements->setCurrentIndex(m_lastTabIndex);
        if (searchTabIndex != -1) {
            m_ui->tabElements->setTabEnabled(searchTabIndex, false);
        }

        m_lastTabIndex = -1;
    } else {
        // On first keystroke, remember which tab was active so we can restore it.
        if (m_lastTabIndex == -1) {
            m_lastTabIndex = m_ui->tabElements->currentIndex();
        }

        // Hide the tab bar so only the unified search results are visible.
        m_ui->tabElements->tabBar()->hide();
        if (searchTabIndex != -1) {
            m_ui->tabElements->setCurrentIndex(searchTabIndex);
            m_ui->tabElements->setTabEnabled(searchTabIndex, true);
        }

        const auto allItems = m_ui->scrollArea_Search->findChildren<ElementLabel *>();

        // First pass: match by object name (e.g. "label_and", "label_or"), which
        // prioritises exact type name hits over looser name-string matches.
        QRegularExpression regex1(QString("^label_.*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        auto searchResults = m_ui->scrollArea_Search->findChildren<ElementLabel *>(regex1);

        // Second pass: match by the human-readable translated element name.
        QRegularExpression regex2(QString(".*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);

        for (auto *item : allItems) {
            if (regex2.match(item->name()).hasMatch()) {
                if (!searchResults.contains(item)) {
                    searchResults.append(item);
                }
            }
        }

        // Third pass: also search IC file names — ICs share the object name
        // "label_ic" so the regex above can't distinguish between them.
        const auto ics = m_ui->scrollArea_Search->findChildren<ElementLabel *>("label_ic");

        for (auto *ic : ics) {
            if (regex2.match(ic->icFileName()).hasMatch()) {
                searchResults.append(ic);
            }
        }

        for (auto *item : allItems) {
            item->setHidden(true);
        }

        for (auto *item : std::as_const(searchResults)) {
            item->setVisible(true);
        }
    }

    m_ui->scrollAreaWidgetContents_Search->layout()->addItem(m_ui->verticalSpacer_Search);
}

void MainWindow::on_lineEditSearch_returnPressed()
{
    if (!m_currentTab) {
        return;
    }

    auto allLabels = m_ui->scrollArea_Search->findChildren<ElementLabel *>();

    for (auto *label : allLabels) {
        if (label->isVisible()) {
            m_currentTab->scene()->addItem(label->mimeData());
            m_ui->lineEditSearch->clear();
            return;
        }
    }
}

void MainWindow::on_actionReloadFile_triggered()
{
    if (!m_currentFile.exists() || !m_currentTab) {
        return;
    }

    const QString file = m_currentFile.absoluteFilePath();

    closeTab(m_tabIndex);
    loadPandaFile(file);
}

void MainWindow::on_actionGates_triggered(const bool checked)
{
    if (!m_currentTab) {
        return;
    }

    // Wire visibility depends on gates being visible: if gates are hidden, wires
    // make no sense and should be hidden too.  Re-enable the wire toggle only when
    // gates are shown so the user can't end up with floating wires.
    m_ui->actionWires->setEnabled(checked);
    // When hiding gates, force wires off regardless of the wire-toggle state.
    // When showing gates, restore whatever the wire toggle was set to.
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

    if (!fileName.endsWith(".ino")) {
        fileName.append(".ino");
    }

    elements = Common::sortGraphicElements(elements);

    ArduinoCodeGen arduino(QDir::home().absoluteFilePath(fileName), elements);
    arduino.generate();
    m_ui->statusBar->showMessage(tr("Arduino code successfully generated."), 4000);

    qCDebug(zero) << "Arduino code successfully generated.";
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
    if (!m_currentTab) {
        return;
    }

    QString path;

    if (m_currentFile.exists()) {
        path = m_currentFile.absolutePath();
    }

    const QString fileName = QFileDialog::getSaveFileName(this, tr("Generate Arduino Code"), path, tr("Arduino file (*.ino)"));

    if (!fileName.isEmpty()) {
        exportToArduino(fileName);
    }
}

void MainWindow::on_actionZoomIn_triggered() const
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->view()->zoomIn();
}

void MainWindow::on_actionZoomOut_triggered() const
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->view()->zoomOut();
}

void MainWindow::on_actionResetZoom_triggered() const
{
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

void MainWindow::updateRecentFileActions()
{
    const auto files = m_recentFiles->recentFiles();
    const int numRecentFiles = static_cast<int>(qMin(files.size(), GlobalProperties::maxRecentFiles));

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

    for (int i = numRecentFiles; i < GlobalProperties::maxRecentFiles; ++i) {
        actions.at(i)->setVisible(false);
    }
}

void MainWindow::openRecentFile()
{
    if (auto *action = qobject_cast<QAction *>(sender())) {
        loadPandaFile(action->data().toString());
    }
}

void MainWindow::createRecentFileActions()
{
    m_ui->menuRecentFiles->clear();

    for (int i = 0; i < GlobalProperties::maxRecentFiles; ++i) {
        auto *action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
        m_ui->menuRecentFiles->addAction(action);
    }

    updateRecentFileActions();
}

void MainWindow::on_actionExportToPdf_triggered()
{
    if (!m_currentTab) {
        return;
    }

    // De-select elements so their selection handles don't appear in the export.
    m_currentTab->scene()->clearSelection();

    QString path;

    if (m_currentFile.exists()) {
        path = m_currentFile.absolutePath();
    }

    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), path, tr("PDF files (*.pdf)"));

    if (pdfFile.isEmpty()) {
        return;
    }

    if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
        pdfFile.append(".pdf");
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    // Landscape fits most circuits better than portrait.
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFile);
    QPainter painter;

    if (!painter.begin(&printer)) {
        throw PANDACEPTION("Could not print this circuit to PDF.");
    }

    auto *scene = m_currentTab->scene();
    // Add a 64 px margin around the bounding rect so elements at the edge aren't clipped.
    scene->render(&painter, QRectF(), scene->itemsBoundingRect().adjusted(-64, -64, 64, 64));
    painter.end();

    m_ui->statusBar->showMessage(tr("Exported file successfully."), 4000);

    QDesktopServices::openUrl(QUrl::fromLocalFile(pdfFile));
}

void MainWindow::on_actionExportToImage_triggered()
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->scene()->clearSelection();

    QString path;

    if (m_currentFile.exists()) {
        path = m_currentFile.absolutePath();
    }

    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), path, tr("PNG files (*.png)"));

    if (pngFile.isEmpty()) {
        return;
    }

    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }

    // Render to a pixmap sized exactly to the circuit bounding box + 64 px margin.
    // Antialiasing is enabled here but not in the PDF path because QPrinter
    // already renders at high DPI.
    QRectF rect = m_currentTab->scene()->itemsBoundingRect().adjusted(-64, -64, 64, 64);
    QPixmap pixmap(rect.size().toSize());
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_currentTab->scene()->render(&painter, QRectF(), rect);
    painter.end();
    pixmap.save(pngFile);

    m_ui->statusBar->showMessage(tr("Exported file successfully."), 4000);

    QDesktopServices::openUrl(QUrl::fromLocalFile(pngFile));
}

void MainWindow::retranslateUi()
{
    ElementFactory::clearCache();
    m_ui->retranslateUi();
    m_ui->elementEditor->retranslateUi();

    const auto items = m_ui->tabElements->findChildren<ElementLabel *>();

    for (auto *item : items) {
        item->updateName();
    }

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
        auto fileInfo = workspace->fileInfo();

        QString text = fileInfo.exists() ? fileInfo.fileName() : tr("New Project");

        if (!undoStack->isClean()) {
            text += "*";
        }

        m_ui->tab->setTabText(index, text);

        for (auto *elm : workspace->scene()->elements()) {
            elm->retranslate();
        }
    }
}

void MainWindow::loadTranslation(const QString &language)
{
    if (language.isEmpty()) {
        return;
    }

    // Persist immediately so if the app crashes during translation loading the
    // preference is still saved for the next launch.
    Settings::setValue("language", language);

    // Always remove and recreate translators rather than calling load() on an
    // existing one — Qt does not guarantee that a re-loaded translator emits
    // languageChanged reliably.
    qApp->removeTranslator(m_pandaTranslator);
    qApp->removeTranslator(m_qtTranslator);

    delete m_pandaTranslator;
    delete m_qtTranslator;

    m_pandaTranslator = nullptr;
    m_qtTranslator = nullptr;

    // English is the source language; no .qm file exists for it, so just
    // retranslate the UI (which resets to the source strings).
    if (language == "en") {
        retranslateUi();
        return;
    }

    // ---------------------------------------------

    // Dynamic translation loading based on available files
    QString pandaFile = QString(":/Translations/wpanda_%1.qm").arg(language);
    QString qtFile = QString(":/Translations/qt_%1.qm").arg(language);

    // Check if wiRedPanda translation exists
    QResource pandaResource(pandaFile);
    if (pandaResource.isValid()) {
        m_pandaTranslator = new QTranslator(this);

        if (!m_pandaTranslator->load(pandaFile) || !qApp->installTranslator(m_pandaTranslator)) {
            qCWarning(zero) << "Failed to load wiRedPanda translation for" << language << ", falling back to English";
            delete m_pandaTranslator;
            m_pandaTranslator = nullptr;
            // Fallback to English
            if (language != "en") {
                loadTranslation("en");
                return;
            }
        }
    }

    // Check if Qt translation exists
    QResource qtResource(qtFile);
    if (qtResource.isValid()) {
        m_qtTranslator = new QTranslator(this);

        if (!m_qtTranslator->load(qtFile) || !qApp->installTranslator(m_qtTranslator)) {
            qCWarning(zero) << "Failed to load Qt translation for" << language << ", continuing without Qt translation";
            delete m_qtTranslator;
            m_qtTranslator = nullptr;
        }
    }

    retranslateUi();
}

QStringList MainWindow::getAvailableLanguages() const
{
    QStringList languages;

    // Always include English as it's the default
    languages << "en";

    // The Qt resource system exposes ":/Translations" as a virtual directory
    // that can be listed like a real filesystem — more robust than hardcoding a
    // language list, and automatically picks up newly added .qm files.
    QDir translationsDir(":/Translations");
    if (translationsDir.exists()) {
        QStringList qmFiles = translationsDir.entryList({"wpanda_*.qm"}, QDir::Files);

        for (const QString &file : qmFiles) {
            // Extract language code from filename (e.g., "wpanda_fr.qm" -> "fr")
            QString langCode = file;
            langCode.remove("wpanda_");
            langCode.remove(".qm");

            // Verify the resource actually exists and is valid
            QString resourcePath = QString(":/Translations/%1").arg(file);
            QResource resource(resourcePath);

            if (resource.isValid() && !langCode.isEmpty()) {
                languages << langCode;
            }
        }
    } else {
        // Fallback: Try all possible language patterns if directory listing fails
        // This covers edge cases where QDir doesn't work with Qt resources
        const QStringList languagePatterns = {
            "ar", "bg", "bn", "cs", "da", "de", "el", "es", "et", "fa", "fi", "fr",
            "he", "hi", "hr", "hu", "id", "it", "ja", "ko", "lt", "lv", "ms", "nb",
            "nl", "pl", "pt", "pt_BR", "ro", "ru", "sk", "sv", "th", "tr", "uk",
            "vi", "zh_Hans", "zh_Hant"
        };

        for (const QString &langCode : languagePatterns) {
            QString resourcePath = QString(":/Translations/wpanda_%1.qm").arg(langCode);
            QResource resource(resourcePath);

            if (resource.isValid()) {
                languages << langCode;
            }
        }
    }

    // Remove duplicates and sort
    languages.removeDuplicates();
    languages.sort();
    return languages;
}

void MainWindow::populateLanguageMenu()
{
    // Clear existing actions from the UI menu
    m_ui->menuLanguage->clear();

    const auto availableLanguages = getAvailableLanguages();
    auto *languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);

    for (const QString &langCode : availableLanguages) {
        auto *action = new QAction(getLanguageDisplayName(langCode), this);
        action->setCheckable(true);
        action->setData(langCode);

        // Set the flag icon for the language
        action->setIcon(QIcon(getLanguageFlagIcon(langCode)));

        // Check if this is the current language
        if (langCode == Settings::value("language").toString() ||
           (langCode == "en" && Settings::value("language").toString().isEmpty())) {
            action->setChecked(true);
        }

        languageGroup->addAction(action);
        m_ui->menuLanguage->addAction(action);

        connect(action, &QAction::triggered, this, [this, langCode]() {
            loadTranslation(langCode);
        });
    }
}

QString MainWindow::getLanguageDisplayName(const QString &langCode) const
{
    // Use Qt's built-in locale system to get native language names
    QLocale locale(langCode);

    // Special handling for some language codes that need specific locales
    if (langCode == "pt_BR") {
        locale = QLocale(QLocale::Portuguese, QLocale::Brazil);
    }

    // Get the native language name (e.g., "Deutsch" for German)
    QString nativeName = locale.nativeLanguageName();

    // If we have a country-specific variant, add the country name
    if (langCode.contains('_') || langCode == "pt_BR") {
        QString nativeCountryName = locale.nativeTerritoryName();
        if (!nativeCountryName.isEmpty() && nativeCountryName != nativeName) {
            nativeName += QString(" (%1)").arg(nativeCountryName);
        }
    }

    // Fallback to language code if Qt doesn't have the name
    if (nativeName.isEmpty()) {
        // Manual fallbacks for languages Qt might not fully support
        static const QMap<QString, QString> fallbackNames = {
            {"bn", "বাংলা"},
            {"fa", "فارسی"},
            {"he", "עברית"},
            {"th", "ไทย"}
        };
        nativeName = fallbackNames.value(langCode, langCode);
    }

    // Ensure proper capitalization for the first letter
    if (!nativeName.isEmpty() && nativeName.at(0).isLetter()) {
        nativeName[0] = nativeName.at(0).toUpper();
    }

    return nativeName;
}

QString MainWindow::getLanguageFlagIcon(const QString &langCode) const
{
    // Use Qt's locale system to determine the appropriate country flag
    QLocale locale(langCode);

    // Special handling for specific language variants
    if (langCode == "pt") {
        locale = QLocale(QLocale::Portuguese, QLocale::Portugal);
    }

    // Map Qt country codes to our flag resource names
    static const QMap<QLocale::Country, QString> countryToFlag = {
        {QLocale::SaudiArabia, ":/Interface/Locale/arabic.svg"},          // Arabic
        {QLocale::Bulgaria, ":/Interface/Locale/bulgarian.svg"},          // Bulgarian
        {QLocale::Bangladesh, ":/Interface/Locale/bangladesh.svg"},       // Bengali
        {QLocale::CzechRepublic, ":/Interface/Locale/czech.svg"},        // Czech
        {QLocale::Denmark, ":/Interface/Locale/danish.svg"},             // Danish
        {QLocale::Germany, ":/Interface/Locale/german.svg"},             // German
        {QLocale::Greece, ":/Interface/Locale/greek.svg"},               // Greek
        {QLocale::UnitedStates, ":/Interface/Locale/usa.svg"},           // English
        {QLocale::Spain, ":/Interface/Locale/spanish.svg"},              // Spanish
        {QLocale::Estonia, ":/Interface/Locale/estonian.svg"},           // Estonian
        {QLocale::Iran, ":/Interface/Locale/iranian.svg"},               // Persian/Farsi
        {QLocale::Finland, ":/Interface/Locale/finnish.svg"},            // Finnish
        {QLocale::France, ":/Interface/Locale/french.svg"},              // French
        {QLocale::Israel, ":/Interface/Locale/hebrew.svg"},              // Hebrew
        {QLocale::India, ":/Interface/Locale/hindi.svg"},                // Hindi
        {QLocale::Croatia, ":/Interface/Locale/croatian.svg"},           // Croatian
        {QLocale::Hungary, ":/Interface/Locale/hungarian.svg"},          // Hungarian
        {QLocale::Indonesia, ":/Interface/Locale/indonesian.svg"},       // Indonesian
        {QLocale::Italy, ":/Interface/Locale/italian.svg"},              // Italian
        {QLocale::Japan, ":/Interface/Locale/japanese.svg"},             // Japanese
        {QLocale::SouthKorea, ":/Interface/Locale/korean.svg"},          // Korean
        {QLocale::Lithuania, ":/Interface/Locale/lithuanian.svg"},       // Lithuanian
        {QLocale::Latvia, ":/Interface/Locale/latvian.svg"},             // Latvian
        {QLocale::Malaysia, ":/Interface/Locale/malaysian.svg"},         // Malay
        {QLocale::Norway, ":/Interface/Locale/norwegian.svg"},           // Norwegian
        {QLocale::Netherlands, ":/Interface/Locale/dutch.svg"},          // Dutch
        {QLocale::Poland, ":/Interface/Locale/polish.svg"},              // Polish
        {QLocale::Portugal, ":/Interface/Locale/portuguese.svg"},        // Portuguese
        {QLocale::Brazil, ":/Interface/Locale/brasil.svg"},              // Portuguese (Brazil)
        {QLocale::Romania, ":/Interface/Locale/romanian.svg"},           // Romanian
        {QLocale::Russia, ":/Interface/Locale/russian.svg"},             // Russian
        {QLocale::Slovakia, ":/Interface/Locale/slovak.svg"},            // Slovak
        {QLocale::Sweden, ":/Interface/Locale/swedish.svg"},             // Swedish
        {QLocale::Thailand, ":/Interface/Locale/thai.svg"},              // Thai
        {QLocale::Turkey, ":/Interface/Locale/turkish.svg"},             // Turkish
        {QLocale::Ukraine, ":/Interface/Locale/ukrainian.svg"},          // Ukrainian
        {QLocale::Vietnam, ":/Interface/Locale/vietnamese.svg"},         // Vietnamese
        {QLocale::China, ":/Interface/Locale/chinese.svg"},              // Chinese Simplified
        {QLocale::Taiwan, ":/Interface/Locale/chinese_traditional.svg"}  // Chinese Traditional
    };

    // Get the flag for the locale's country
    QString flagIcon = countryToFlag.value(locale.territory(), ":/Interface/Locale/default.svg");

    // Fallback for languages where Qt might not detect the country correctly
    if (flagIcon == ":/Interface/Locale/default.svg") {
        static const QMap<QString, QString> languageFallbacks = {
            {"ar", ":/Interface/Locale/arabic.svg"},
            {"bg", ":/Interface/Locale/bulgarian.svg"},
            {"bn", ":/Interface/Locale/bangladesh.svg"},
            {"cs", ":/Interface/Locale/czech.svg"},
            {"da", ":/Interface/Locale/danish.svg"},
            {"de", ":/Interface/Locale/german.svg"},
            {"el", ":/Interface/Locale/greek.svg"},
            {"en", ":/Interface/Locale/usa.svg"},
            {"es", ":/Interface/Locale/spanish.svg"},
            {"et", ":/Interface/Locale/estonian.svg"},
            {"fa", ":/Interface/Locale/iranian.svg"},
            {"fi", ":/Interface/Locale/finnish.svg"},
            {"fr", ":/Interface/Locale/french.svg"},
            {"he", ":/Interface/Locale/hebrew.svg"},
            {"hi", ":/Interface/Locale/hindi.svg"},
            {"hr", ":/Interface/Locale/croatian.svg"},
            {"hu", ":/Interface/Locale/hungarian.svg"},
            {"id", ":/Interface/Locale/indonesian.svg"},
            {"it", ":/Interface/Locale/italian.svg"},
            {"ja", ":/Interface/Locale/japanese.svg"},
            {"ko", ":/Interface/Locale/korean.svg"},
            {"lt", ":/Interface/Locale/lithuanian.svg"},
            {"lv", ":/Interface/Locale/latvian.svg"},
            {"ms", ":/Interface/Locale/malaysian.svg"},
            {"nb", ":/Interface/Locale/norwegian.svg"},
            {"nl", ":/Interface/Locale/dutch.svg"},
            {"pl", ":/Interface/Locale/polish.svg"},
            {"pt", ":/Interface/Locale/portuguese.svg"},
            {"pt_BR", ":/Interface/Locale/brasil.svg"},
            {"ro", ":/Interface/Locale/romanian.svg"},
            {"ru", ":/Interface/Locale/russian.svg"},
            {"sk", ":/Interface/Locale/slovak.svg"},
            {"sv", ":/Interface/Locale/swedish.svg"},
            {"th", ":/Interface/Locale/thai.svg"},
            {"tr", ":/Interface/Locale/turkish.svg"},
            {"uk", ":/Interface/Locale/ukrainian.svg"},
            {"vi", ":/Interface/Locale/vietnamese.svg"},
            {"zh_Hans", ":/Interface/Locale/chinese.svg"},
            {"zh_Hant", ":/Interface/Locale/chinese_traditional.svg"}
        };
        flagIcon = languageFallbacks.value(langCode, ":/Interface/Locale/default.svg");
    }

    return flagIcon;
}

void MainWindow::on_actionPlay_toggled(const bool checked)
{
    if (!m_currentTab) {
        return;
    }

    auto *simulation = m_currentTab->simulation();

    // toggled(bool) carries the new checked state, so this is the canonical
    // start/stop dispatch point — no separate on/off slots needed.
    checked ? simulation->start() : simulation->stop();
}

void MainWindow::on_actionRestart_triggered()
{
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

void MainWindow::populateLeftMenu()
{
    // Set to the first tab (Inputs/Outputs)
    const int ioTabIndex = getTabIndex("io");
    if (ioTabIndex != -1) {
        m_ui->tabElements->setCurrentIndex(ioTabIndex);
    } else {
        // Fallback to first tab if object name not found
        m_ui->tabElements->setCurrentIndex(0);
    }
    populateMenu(m_ui->verticalSpacer_InOut, {"InputVcc", "InputGnd", "InputButton", "InputSwitch", "InputRotary", "Clock", "Led", "Display7", "Display14", "Display16", "Buzzer", "AudioBox"}, m_ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(m_ui->verticalSpacer_Gates, {"And", "Or", "Not", "Nand", "Nor", "Xor", "Xnor", "Node"}, m_ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(m_ui->verticalSpacer_Combinational, {"TruthTable", "Mux", "Demux"}, m_ui->scrollAreaWidgetContents_Combinational->layout());
    populateMenu(m_ui->verticalSpacer_Memory, {"DLatch", "SRLatch", "DFlipFlop", "JKFlipFlop", "TFlipFlop"}, m_ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(m_ui->verticalSpacer_Misc, {"Text", "Line"}, m_ui->scrollAreaWidgetContents_Misc->layout());
}

void MainWindow::on_actionFastMode_triggered(const bool checked)
{
    setFastMode(checked);
    Settings::setValue("fastMode", checked);
}

void MainWindow::on_actionWaveform_triggered()
{
    if (!m_currentTab) {
        return;
    }

    qCDebug(zero) << "BD fileName: " << m_currentTab->dolphinFileName();
    auto *bewavedDolphin = new BewavedDolphin(m_currentTab->scene(), true, this);
    bewavedDolphin->createWaveform(m_currentTab->dolphinFileName());
    bewavedDolphin->show();
}

void MainWindow::on_actionLightTheme_triggered()
{
    ThemeManager::setTheme(Theme::Light);
}

void MainWindow::on_actionDarkTheme_triggered()
{
    ThemeManager::setTheme(Theme::Dark);
}

void MainWindow::updateTheme()
{
    // Pixmaps are theme-dependent (dark/light SVG variants); clearing the cache
    // forces each element to re-render with the new palette on next paint.
    ElementFactory::clearCache();

    switch (ThemeManager::theme()) {
    case Theme::Dark:  m_ui->actionDarkTheme->setChecked(true); break;
    case Theme::Light: m_ui->actionLightTheme->setChecked(true); break;

    default:
        // Handle unexpected theme values - fallback to Light theme
        m_ui->actionLightTheme->setChecked(true);
        break;
    }

    // Memory tab icon uses DFlipFlop's SVG which differs between themes.
    const int memoryTabIndex = getTabIndex("memory");
    if (memoryTabIndex != -1) {
        m_ui->tabElements->setTabIcon(memoryTabIndex, QIcon(DFlipFlop::pixmapPath()));
    }

    // Only memory element labels are theme-sensitive (their SVGs change colour);
    // other tabs use fixed icons and don't need individual repaints.
    const auto labels = m_ui->memory->findChildren<ElementLabel *>();

    for (auto *label : labels) {
        label->updateTheme();
    }

    m_ui->elementEditor->updateTheme();
}

void MainWindow::on_actionFlipHorizontally_triggered()
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->scene()->flipHorizontally();
}

void MainWindow::on_actionFlipVertically_triggered()
{
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
    isFullScreen() ? showNormal() : showFullScreen();
}

void MainWindow::on_actionMute_triggered(const bool checked)
{
    if (!m_currentTab) {
        return;
    }

    m_currentTab->scene()->mute(checked);
    m_ui->actionMute->setText(checked ? tr("Unmute") : tr("Mute"));
}

void MainWindow::on_actionLabelsUnderIcons_triggered(const bool checked)
{
    m_ui->mainToolBar->setToolButtonStyle(checked ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    Settings::setValue("labelsUnderIcons", checked);
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
    if (!m_currentTab) {
        return;
    }

    // The IC list is directory-relative.  If the project hasn't been saved yet
    // we don't have a directory to copy into, so require a save first.
    if (!m_currentTab->fileInfo().isReadable()) {
        throw PANDACEPTION("Save file first.");
    }

    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Open File"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("Panda (*.panda)"));

    if (fileDialog.exec() == QDialog::Rejected) {
        return;
    }

    const auto files = fileDialog.selectedFiles();

    if (files.isEmpty()) {
        return;
    }

    QMessageBox::information(this, tr("Info"), tr("Selected files (and their dependencies) will be copied to current file folder."));

    // Copy the chosen .panda file (and any ICs it depends on transitively)
    // into the project's directory so that relative paths work when reopened.
    for (const auto &file : files) {
        QFileInfo destPath(GlobalProperties::currentDir + "/" + QFileInfo(file).fileName());
        IC::copyFiles(QFileInfo(file), destPath);
    }

    updateICList();
}

void MainWindow::on_pushButtonRemoveIC_clicked()
{
    QMessageBox::information(this, tr("Info"), tr("Drag here to remove."));
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
    // label().append(".panda").toLower() reconstructs the filename from the IC
    // label (e.g. "adder" → "adder.panda") for comparison against icFileName.
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        if ((*it)->elementType() == ElementType::IC && (*it)->label().append(".panda").toLower() == icFileName) {
            m_currentTab->scene()->removeItem(*it);
            delete *it;
        }
    }

    QFile file(GlobalProperties::currentDir + "/" + icFileName);

    if (!file.remove()) {
        throw PANDACEPTION("Error removing file: %1", file.errorString());
    }

    updateICList();
    // Auto-save after removal so the scene no longer references the deleted file.
    on_actionSave_triggered();
}

int MainWindow::getTabIndex(const QString &objectName) const
{
    for (int i = 0; i < m_ui->tabElements->count(); ++i) {
        QWidget *tabWidget = m_ui->tabElements->widget(i);
        if (tabWidget && tabWidget->objectName() == objectName) {
            return i;
        }
    }
    return -1;
}

