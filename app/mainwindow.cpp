// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "bewaveddolphin.h"
#include "codegenerator.h"
#include "common.h"
#include "dflipflop.h"
#include "elementfactory.h"
#include "elementlabel.h"
#include "globalproperties.h"
#include "graphicsview.h"
#include "ic.h"
#include "recentfiles.h"
#include "settings.h"
#include "simulation.h"
#include "simulationblocker.h"
#include "thememanager.h"
#include "workspace.h"

#include <QActionGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPdfWriter>
#include <QPrinter>
#include <QSaveFile>
#include <QShortcut>
#include <QKeySequence>
#include <QTemporaryFile>
#include <QTranslator>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#define SKIPEMPTYPARTS QString::SkipEmptyParts
#else
#define SKIPEMPTYPARTS Qt::SkipEmptyParts
#endif

MainWindow::MainWindow(const QString &fileName, QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    qCDebug(zero) << tr("WiRedPanda Version = ") << APP_VERSION << tr(" OR ") << GlobalProperties::version;
    m_ui->setupUi(this);

    qCDebug(zero) << tr("Settings fileName: ") << Settings::fileName();
    loadTranslation(Settings::value("language").toString());

    connect(m_ui->tab, &QTabWidget::currentChanged,    this, &MainWindow::tabChanged);
    connect(m_ui->tab, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    qCDebug(zero) << tr("Restoring geometry and setting zoom controls.");
    restoreGeometry(Settings::value("MainWindow/geometry").toByteArray());
    restoreState(Settings::value("MainWindow/windowState").toByteArray());
    m_ui->splitter->restoreGeometry(Settings::value("MainWindow/splitter/geometry").toByteArray());
    m_ui->splitter->restoreState(Settings::value("MainWindow/splitter/state").toByteArray());

    qCDebug(zero) << tr("Preparing theme and UI modes.");
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

    qCDebug(zero) << tr("Setting left side menus.");
    auto *searchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(searchShortcut, &QShortcut::activated, m_ui->lineEditSearch, qOverload<>(&QWidget::setFocus));
    populateLeftMenu();
    m_ui->tabElements->setTabEnabled(6, false);

    qCDebug(zero) << tr("Loading recent file list.");
    m_recentFiles = new RecentFiles(this);
    connect(this, &MainWindow::addRecentFile, m_recentFiles, &RecentFiles::addRecentFile);
    createRecentFileActions();
    connect(m_recentFiles, &RecentFiles::recentFilesUpdated, this, &MainWindow::updateRecentFileActions);

    qCDebug(zero) << tr("Checking playing simulation.");
    m_ui->actionPlay->setChecked(true);

    qCDebug(zero) << tr("Window title.");
    setWindowTitle("WiRedPanda " + QString(APP_VERSION));

    qCDebug(zero) << tr("Building a new tab.");
    createNewTab();

    qCDebug(zero) << tr("Opening file if not empty.");
    if (!fileName.isEmpty()) {
        loadPandaFile(fileName);
    }

    qCDebug(zero) << tr("Disabling Arduino export.");
    m_ui->actionExportToArduino->setEnabled(false);

    QPixmapCache::setCacheLimit(100'000);

    qCDebug(zero) << tr("Adding examples to menu");
    QDir examplesDir("examples");

    if (examplesDir.exists()) {
        const auto entryList = examplesDir.entryList({"*.panda"}, QDir::Files);

        for (const auto &entry : entryList) {
            auto *action = new QAction(entry);

            connect(action, &QAction::triggered, this, [this] {
                if (auto *action = qobject_cast<QAction *>(sender())) {
                    loadPandaFile("examples/" + action->text());
                }});

            m_ui->menuExamples->addAction(action);
        }
    }

    if (m_ui->menuExamples->isEmpty()) {
        m_ui->menuExamples->menuAction()->setVisible(false);
    }

    // Element shortcuts

    auto *removeInPortShortcut = new QShortcut(QKeySequence("["), this);
    auto *addInPortShortcut = new QShortcut(QKeySequence("]"), this);
    auto *removeOutPortShortcut = new QShortcut(QKeySequence("{"), this);
    auto *addOutPortShortcut = new QShortcut(QKeySequence("}"), this);
    auto *changePrevElmShortcut = new QShortcut(QKeySequence("<"), this);
    auto *changeNextElmShortcut = new QShortcut(QKeySequence(">"), this);

    connect(addInPortShortcut,            &QShortcut::activated,      m_currentTab->scene(), &Scene::addInputPort);
    connect(removeInPortShortcut,         &QShortcut::activated,      m_currentTab->scene(), &Scene::removeInputPort);
    connect(addOutPortShortcut,           &QShortcut::activated,      m_currentTab->scene(), &Scene::addOutputPort);
    connect(removeOutPortShortcut,        &QShortcut::activated,      m_currentTab->scene(), &Scene::removeOutputPort);
    connect(changePrevElmShortcut,        &QShortcut::activated,      m_currentTab->scene(), &Scene::prevElm);
    connect(changeNextElmShortcut,        &QShortcut::activated,      m_currentTab->scene(), &Scene::nextElm);

    qCDebug(zero) << tr("Setting connections");
    connect(m_ui->actionShortcutsAndTips, &QAction::triggered,        this,                &MainWindow::on_actionShortcuts_and_Tips_triggered);
    connect(m_ui->actionAbout,            &QAction::triggered,        this,                &MainWindow::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,          &QAction::triggered,        this,                &MainWindow::on_actionAboutQt_triggered);
    connect(m_ui->actionAboutThisVersion, &QAction::triggered,        this,                &MainWindow::aboutThisVersion);
    connect(m_ui->actionChangeTrigger,    &QAction::triggered,        m_ui->elementEditor, &ElementEditor::changeTriggerAction);
    connect(m_ui->actionDarkTheme,        &QAction::triggered,        this,                &MainWindow::on_actionDarkTheme_triggered);
    connect(m_ui->actionEnglish,          &QAction::triggered,        this,                &MainWindow::on_actionEnglish_triggered);
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
    connect(m_ui->actionPortuguese,       &QAction::triggered,        this,                &MainWindow::on_actionPortuguese_triggered);
    connect(m_ui->actionReloadFile,       &QAction::triggered,        this,                &MainWindow::on_actionReloadFile_triggered);
    connect(m_ui->actionRename,           &QAction::triggered,        m_ui->elementEditor, &ElementEditor::renameAction);
    connect(m_ui->actionResetZoom,        &QAction::triggered,        this,                &MainWindow::on_actionResetZoom_triggered);
    connect(m_ui->actionRestart,          &QAction::triggered,        this,                &MainWindow::on_actionRestart_triggered);
    connect(m_ui->actionRotateLeft,       &QAction::triggered,        this,                &MainWindow::on_actionRotateLeft_triggered);
    connect(m_ui->actionRotateRight,      &QAction::triggered,        this,                &MainWindow::on_actionRotateRight_triggered);
    connect(m_ui->actionSave,             &QAction::triggered,        this,                &MainWindow::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,           &QAction::triggered,        this,                &MainWindow::on_actionSaveAs_triggered);
    connect(m_ui->actionSelectAll,        &QAction::triggered,        this,                &MainWindow::on_actionSelectAll_triggered);
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
    delete m_ui;
}

void MainWindow::loadAutosaveFiles()
{
    QStringList autosaves(Settings::value("autosaveFile").toStringList());

    qCDebug(zero) << tr("All autosave files: ") << autosaves;

    for (auto it = autosaves.begin(); it != autosaves.end();) {
        QFile file(*it);

        if (!file.exists()) {
            qCDebug(zero) << tr("Removing from config the autosave file that does not exist.");
            it = autosaves.erase(it);
            continue;
        }

        try {
            loadPandaFile(*it);
        } catch (const std::exception &e) {
            QMessageBox::critical(nullptr, tr("Error!"), e.what());
            qCDebug(zero) << tr("Removing autosave file that is corrupted.");
            it = autosaves.erase(it);
            continue;
        }

        m_currentTab->setAutosaveFile();

        ++it;
    }

    Settings::setValue("autosaveFile", autosaves);
}

void MainWindow::createNewTab()
{
    qCDebug(zero) << tr("Creating new workspace.");
    auto *workspace = new WorkSpace(this);

    connect(workspace, &WorkSpace::fileChanged, this, &MainWindow::setCurrentFile);

    workspace->view()->setFastMode(m_ui->actionFastMode->isChecked());
    workspace->scene()->updateTheme();

    qCDebug(zero) << tr("Adding tab. #tabs: ") << m_ui->tab->count() << tr(", current tab: ") << m_tabIndex;
    m_ui->tab->addTab(workspace, tr("New Project"));

    qCDebug(zero) << tr("Selecting the newly created tab.");
    m_ui->tab->setCurrentIndex(m_ui->tab->count() - 1);

    qCDebug(zero) << tr("Finished #tabs: ") << m_ui->tab->count() << tr(", current tab: ") << m_tabIndex;
}

void MainWindow::removeUndoRedoMenu()
{
    if (!m_currentTab) {
        return;
    }

    m_ui->menuEdit->removeAction(m_ui->menuEdit->actions().constFirst());
    m_ui->menuEdit->removeAction(m_ui->menuEdit->actions().constFirst());
}

void MainWindow::addUndoRedoMenu()
{
    m_ui->menuEdit->insertAction(m_ui->menuEdit->actions().at(0), m_currentTab->scene()->undoAction());
    m_ui->menuEdit->insertAction(m_ui->menuEdit->actions().at(1), m_currentTab->scene()->redoAction());
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

    if (!Settings::contains("hideV4Warning")) {
        aboutThisVersion();
    }

    qCDebug(zero) << tr("Checking for autosave file recovery.");
    loadAutosaveFiles();
}

void MainWindow::aboutThisVersion()
{
    qCDebug(zero) << tr("'hideV4Warning' message box.");

    auto *checkBox = new QCheckBox(tr("Don't show this again."));

    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.setWindowTitle("WiRedPanda " + QString(APP_VERSION));
    msgBox.setText(
        tr("WiRedPanda version >= 4.0 is not 100% compatible with previous versions.\n"
           "To open old version projects containing ICs (or boxes), skins, and/or "
           "beWavedDolphin simulations, their files must be moved to the same directory "
           "as the main project file.\n"
           "WiRedPanda %1 will automatically list all other .panda files located "
           "in the same directory of the current project as ICs in the editor tab.\n"
           "You have to save new projects before accessing ICs and skins, or running "
           "beWavedDolphin simulations.")
            .arg(APP_VERSION));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setCheckBox(checkBox);

    connect(checkBox, &QCheckBox::stateChanged, this, [](int state) {
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

    if (multiple) {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
    } else {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    }

    const QString fileName = m_currentFile.fileName().isEmpty() ? tr("New Project") : m_currentFile.fileName();

    msgBox.setText(fileName + tr(" has been modified. \nDo you want to save your changes?"));
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
    qCDebug(zero) << tr("Loading in editor.");
    m_currentTab->load(fileName);
    updateICList();
    m_ui->statusBar->showMessage(tr("File loaded successfully."), 4000);
}

void MainWindow::on_actionOpen_triggered()
{
    const QString path = m_currentFile.exists() ? "" : "./examples";
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), path, tr("Panda files (*.panda)"));

    if (fileName.isEmpty()) {
        return;
    }

    loadPandaFile(fileName);
}

void MainWindow::on_actionSave_triggered()
{
    // TODO: if current file is autosave ask for filename

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
    QString path;

    if (!m_currentFile.fileName().isEmpty()) {
        path = m_currentFile.absoluteFilePath();
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File as ..."), path, tr("Panda files (*.panda)"));

    if (fileName.isEmpty()) {
        return;
    }

    if (!fileName.endsWith(".panda")) {
        fileName.append(".panda");
    }

    save(fileName);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(
        this,
        "WiRedPanda",
        tr("<p>WiRedPanda is a software developed by the students of the Federal University of São Paulo."
           " This project was created in order to help students learn about logic circuits.</p>"
           "<p>Software version: %1</p>"
           "<p><strong>Creators:</strong></p>"
           "<ul>"
           "<li> Davi Morales </li>"
           "<li> Lucas Lellis </li>"
           "<li> Rodrigo Torres </li>"
           "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
           "</ul>"
           "<p> WiRedPanda is currently maintained by Prof. Fábio Cappabianco, Ph.D. and Vinícius R. Miguel.</p>"
           "<p> Please file a report at our GitHub page if bugs are found or if you wish for a new functionality to be implemented.</p>"
           "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
            .arg(QApplication::applicationVersion()));
}

void MainWindow::on_actionShortcuts_and_Tips_triggered()
{
    QMessageBox::information(this,
        tr("Shortcuts and Tips"),
        tr("<h1>Element Shortcuts</h1>"
           "<ul>"
           "<li> [ : Remove one input port </li>"
           "<li> ] : Add one input port </li>"
           "<li> { : Remove one output port </li>"
           "<li> } : Add one output port </li>"
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

bool MainWindow::closeFiles()
{
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
        auto *undoStack = workspace->scene()->undoStack();

        if (!undoStack->isClean()) {
            return true;
        }

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
    m_currentFile = fileInfo;

    QString text = fileInfo.exists() ? fileInfo.fileName() : tr("New Project");

    if (!m_currentTab->scene()->undoStack()->isClean()) {
        text += "*";
    }

    m_ui->tab->setTabText(m_tabIndex, text);

    qCDebug(zero) << tr("Adding file to recent files.");
    emit addRecentFile(fileInfo.absoluteFilePath());
}

void MainWindow::on_actionSelectAll_triggered()
{
    m_currentTab->scene()->selectAll();
}

void MainWindow::updateICList()
{
    m_ui->scrollAreaWidgetContents_IC->layout()->removeItem(m_ui->verticalSpacer_IC);

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
        qCDebug(zero) << tr("Show files.");
        QDir directory(m_currentFile.absoluteDir());
        QStringList files = directory.entryList({"*.panda", "*.PANDA"}, QDir::Files);
        files.removeAll(m_currentFile.fileName());

        for (int i = files.size() - 1; i >= 0; --i) {
            if (files.at(i).at(0) == '.') {
                files.removeAt(i);
            }
        }

        qCDebug(zero) << tr("Files: ") << files.join(", ");
        for (const QString &file : qAsConst(files)) {
            QPixmap pixmap(":/basic/ic-panda.svg");

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
    qCDebug(zero) << tr("Closing tab ") << tabIndex + 1 << tr(", #tabs: ") << m_ui->tab->count();
    m_ui->tab->setCurrentIndex(tabIndex);

    qCDebug(zero) << tr("Checking if needs to save file.");

    if (!m_currentTab->scene()->undoStack()->isClean()) {
        const int selectedButton = confirmSave(false);

        if (selectedButton == QMessageBox::Cancel) {
            return false;
        }

        if (selectedButton == QMessageBox::Yes) {
            try {
                save();
            } catch (const std::exception &e) {
                QMessageBox::critical(this, tr("Error"), e.what());

                if (closeTabAnyway() == QMessageBox::No) {
                    return false;
                }
            }
        }
    }

    qCDebug(zero) << tr("Deleting tab.");
    m_currentTab->deleteLater();
    m_ui->tab->removeTab(tabIndex);

    qCDebug(zero) << tr("Closed tab ") << tabIndex << tr(", #tabs: ") << m_ui->tab->count() << tr(", current tab: ") << m_tabIndex;

    return true;
}

void MainWindow::disconnectTab()
{
    if (!m_currentTab) {
        return;
    }

    m_ui->elementEditor->setScene(nullptr);

    qCDebug(zero) << tr("Stopping simulation.");
    m_currentTab->simulation()->stop();

    qCDebug(zero) << tr("Disconnecting zoom from UI.");
    disconnect(m_currentTab->view(), &GraphicsView::zoomChanged, this, &MainWindow::zoomChanged);

    qCDebug(zero) << tr("Removing undo and redo actions from UI menu.");
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
    qCDebug(zero) << tr("Connecting undo and redo functions to UI menu.");
    addUndoRedoMenu();

    qCDebug(zero) << tr("Setting Panda file info.");
    m_currentFile = m_currentTab->fileInfo();

    updateICList();

    qCDebug(zero) << tr("Connecting current tab to element editor menu in UI.");
    m_ui->elementEditor->setScene(m_currentTab->scene());

    connect(m_currentTab->view(),       &GraphicsView::zoomChanged, this,                  &MainWindow::zoomChanged);
    connect(m_ui->actionClearSelection, &QAction::triggered,        m_currentTab->scene(), &Scene::clearSelection);
    connect(m_ui->actionCopy,           &QAction::triggered,        m_currentTab->scene(), &Scene::copyAction);
    connect(m_ui->actionCut,            &QAction::triggered,        m_currentTab->scene(), &Scene::cutAction);
    connect(m_ui->actionDelete,         &QAction::triggered,        m_currentTab->scene(), &Scene::deleteAction);
    connect(m_ui->actionPaste,          &QAction::triggered,        m_currentTab->scene(), &Scene::pasteAction);

    if (m_ui->actionPlay->isChecked()) {
        qCDebug(zero) << tr("Restarting simulation.");
        m_currentTab->simulation()->start();
        m_currentTab->scene()->clearSelection();
    }

    m_currentTab->view()->setFastMode(m_ui->actionFastMode->isChecked());
    m_ui->actionZoomIn->setEnabled(m_currentTab->view()->canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_currentTab->view()->canZoomOut());
}

void MainWindow::tabChanged(const int newTabIndex)
{
    disconnectTab(); // disconnect previously selected tab
    m_tabIndex = newTabIndex;
    m_ui->elementEditor->hide();

    if (newTabIndex == -1) {
        m_currentTab = nullptr;
        return;
    }

    m_currentTab = qobject_cast<WorkSpace *>(m_ui->tab->currentWidget());
    qCDebug(zero) << tr("Selecting tab: ") << newTabIndex;
    connectTab();
    qCDebug(zero) << tr("New tab selected. Dolphin fileName: ") << m_currentTab->dolphinFileName();
}

void MainWindow::on_lineEditSearch_textChanged(const QString &text)
{
    m_ui->scrollAreaWidgetContents_Search->layout()->removeItem(m_ui->verticalSpacer_Search);

    if (text.isEmpty()) {
        m_ui->tabElements->tabBar()->show();
        m_ui->tabElements->setCurrentIndex(m_lastTabIndex);
        m_ui->tabElements->setTabEnabled(6, false);

        m_lastTabIndex = -1;
    } else {
        if (m_lastTabIndex == -1) {
            m_lastTabIndex = m_ui->tabElements->currentIndex();
        }

        m_ui->tabElements->tabBar()->hide();
        m_ui->tabElements->setCurrentIndex(6);
        m_ui->tabElements->setTabEnabled(6, true);

        const auto allItems = m_ui->scrollArea_Search->findChildren<ElementLabel *>();

        QRegularExpression regex1(QString("^label_.*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        auto searchResults = m_ui->scrollArea_Search->findChildren<ElementLabel *>(regex1);

        QRegularExpression regex2(QString(".*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);

        for (auto *item : allItems) {
            if (regex2.match(item->name()).hasMatch()) {
                if (!searchResults.contains(item)) {
                    searchResults.append(item);
                }
            }
        }

        const auto ics = m_ui->scrollArea_Search->findChildren<ElementLabel *>("label_ic");

        for (auto *ic : ics) {
            if (regex2.match(ic->icFileName()).hasMatch()) {
                searchResults.append(ic);
            }
        }

        for (auto *item : allItems) {
            item->setHidden(true);
        }

        for (auto *item : qAsConst(searchResults)) {
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
        throw Pandaception(tr("Missing file name."));
    }

    auto elements = m_currentTab->scene()->elements();

    if (elements.isEmpty()) {
        throw Pandaception(tr("The panda file is empty."));
    }

    SimulationBlocker simulationBlocker(m_currentTab->simulation());

    if (!fileName.endsWith(".ino")) {
        fileName.append(".ino");
    }

    elements = Common::sortGraphicElements(elements);

    CodeGenerator arduino(QDir::home().absoluteFilePath(fileName), elements);
    arduino.generate();
    m_ui->statusBar->showMessage(tr("Arduino code successfully generated."), 4000);

    qCDebug(zero) << tr("Arduino code successfully generated.");
}

void MainWindow::exportToWaveFormFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        throw Pandaception(tr("Missing file name."));
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
    const int numRecentFiles = qMin(files.size(), GlobalProperties::maxRecentFiles);

    if (numRecentFiles > 0) {
        m_ui->menuRecentFiles->setEnabled(true);
    }

    auto actions = m_ui->menuRecentFiles->actions();

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
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFile);
    QPainter painter;

    if (!painter.begin(&printer)) {
        throw Pandaception(tr("Could not print this circuit to PDF."));
    }

    auto *scene = m_currentTab->scene();
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
    m_ui->retranslateUi(this);
    m_ui->elementEditor->retranslateUi();

    const auto items = m_ui->tabElements->findChildren<ElementLabel *>();

    for (auto *item : items) {
        item->updateName();
    }

    for (int index = 0; index < m_ui->tab->count(); ++index) {
        auto *workspace = qobject_cast<WorkSpace *>(m_ui->tab->widget(index));
        auto *undoStack = workspace->scene()->undoStack();
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

    Settings::setValue("language", language);

    qApp->removeTranslator(m_pandaTranslator);
    qApp->removeTranslator(m_qtTranslator);

    delete m_pandaTranslator;
    delete m_qtTranslator;

    m_pandaTranslator = nullptr;
    m_qtTranslator = nullptr;

    if (language == "en") {
        retranslateUi();
        return;
    }

    // ---------------------------------------------

    QString pandaFile;
    QString qtFile;

    if (language == "pt_BR") {
        pandaFile = ":/translations/wpanda_pt_BR.qm";
        qtFile = ":/translations/qt_pt_BR.qm";
    }

    if (!pandaFile.isEmpty()) {
        m_pandaTranslator = new QTranslator(this);

        if (!m_pandaTranslator->load(pandaFile) || !qApp->installTranslator(m_pandaTranslator)) {
            throw Pandaception(tr("Error loading WiRedPanda translation!"));
        }
    }

    if (!qtFile.isEmpty()) {
        m_qtTranslator = new QTranslator(this);

        if (!m_qtTranslator->load(qtFile) || !qApp->installTranslator(m_qtTranslator)) {
            throw Pandaception(tr("Error loading Qt translation!"));
        }
    }

    retranslateUi();
}

void MainWindow::on_actionEnglish_triggered()
{
    loadTranslation("en");
}

void MainWindow::on_actionPortuguese_triggered()
{
    loadTranslation("pt_BR");
}

void MainWindow::on_actionPlay_toggled(const bool checked)
{
    if (!m_currentTab) {
        return;
    }

    auto *simulation = m_currentTab->simulation();

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
    layout->removeItem(spacer);

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
    m_ui->tabElements->setCurrentIndex(0);
    populateMenu(m_ui->verticalSpacer_InOut, {"InputVcc", "InputGnd", "InputButton", "InputSwitch", "InputRotary", "Clock", "Led", "Display7", "Display14", "Buzzer"}, m_ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(m_ui->verticalSpacer_Gates, {"And", "Or", "Not", "Nand", "Nor", "Xor", "Xnor", "Mux", "Demux", "Node"}, m_ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(m_ui->verticalSpacer_Memory, {"DLatch", "DFlipFlop", "JKFlipFlop", "SRFlipFlop", "TFlipFlop"}, m_ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(m_ui->verticalSpacer_Misc, {"Text", "Line"}, m_ui->scrollAreaWidgetContents_Misc->layout());
    populateMenu(m_ui->verticalSpacer_Truthtable, {"TruthTable"}, m_ui->scrollAreaWidgetContents_Truthtable->layout());
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

    qCDebug(zero) << tr("BD fileName: ") << m_currentTab->dolphinFileName();
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
    switch (ThemeManager::theme()) {
    case Theme::Dark:  m_ui->actionDarkTheme->setChecked(true); break;
    case Theme::Light: m_ui->actionLightTheme->setChecked(true); break;
    }

    m_ui->tabElements->setTabIcon(2, QIcon(DFlipFlop::pixmapPath()));

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
        if (m_ui->actionPlay->isChecked()) {
            on_actionPlay_toggled(true);
        }
        break;
    }

    case QEvent::WindowDeactivate: {
        on_actionPlay_toggled(false);
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

    if (!m_currentTab->fileInfo().isReadable()) {
        throw Pandaception(tr("Save file first."));
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

    for (const auto &file : files) {
        IC::copyFiles(QFileInfo(file));
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

    SimulationBlocker blocker(m_currentTab->simulation());

    auto elements = m_currentTab->scene()->elements();

    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        if ((*it)->elementType() == ElementType::IC && (*it)->label().append(".panda").toLower() == icFileName) {
            m_currentTab->scene()->removeItem(*it);
            delete *it;
        }
    }

    QFile file(GlobalProperties::currentDir + "/" + icFileName);

    if (!file.remove()) {
        throw Pandaception(tr("Error removing file: ") + file.errorString());
    }

    updateICList();
    on_actionSave_triggered();
}
