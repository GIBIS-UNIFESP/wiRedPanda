// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "bewaveddolphin.h"
#include "codegenerator.h"
#include "common.h"
#include "elementfactory.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicsview.h"
#include "icmanager.h"
#include "label.h"
#include "listitemwidget.h"
#include "recentfilescontroller.h"
#include "settings.h"
#include "simulationcontroller.h"
#include "thememanager.h"
#include "workspace.h"

#include <QActionGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPdfWriter>
#include <QPrinter>
#include <QSaveFile>
#include <QShortcut>
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
    qCDebug(zero) << "WiRedPanda Version =" << APP_VERSION << "OR" << GlobalProperties::version;
    m_ui->setupUi(this);

    qCDebug(zero) << "Settings fileName:" << Settings::fileName();
    loadTranslation(Settings::value("language").toString());

    connect(m_ui->tab, &QTabWidget::currentChanged, this, &MainWindow::selectTab);
    connect(m_ui->tab, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    qCDebug(zero) << "Restoring geometry and setting zoom controls.";
    Settings::beginGroup("MainWindow");
    restoreGeometry(Settings::value("geometry").toByteArray());
    restoreState(Settings::value("windowState").toByteArray());
    Settings::beginGroup("splitter");
    m_ui->splitter->restoreGeometry(Settings::value("geometry").toByteArray());
    m_ui->splitter->restoreState(Settings::value("state").toByteArray());
    m_ui->actionExportToArduino->setEnabled(false);
    Settings::endGroup();
    Settings::endGroup();

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

    qCDebug(zero) << "Setting left side menus.";
    m_ui->searchScrollArea->hide();
    auto *shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(shortcut, &QShortcut::activated, m_ui->lineEdit, qOverload<>(&QWidget::setFocus));
    populateLeftMenu();

    qCDebug(zero) << "Loading recent file list.";
    m_recentFilesController = new RecentFilesController("recentFileList", true, this);
    connect(this, &MainWindow::addRecentFile, m_recentFilesController, &RecentFilesController::addRecentFile);
    m_firstResult = nullptr;
    // updateRecentICs();
    createRecentFileActions();
    connect(m_recentFilesController, &RecentFilesController::recentFilesUpdated, this, &MainWindow::updateRecentFileActions);

    qCDebug(zero) << "Checking for autosave file recovery.";
    loadAutosaveFiles(fileName);
    qCDebug(zero) << "Checking playing simulation.";
    m_ui->actionPlay->setChecked(true);

    qCDebug(zero) << "Connecting ICManager to MainWindow";
    connect(&ICManager::instance(), &ICManager::openIC, this, &MainWindow::loadPandaFile);

    qCDebug(zero) << "Setting connections";
    connect(m_ui->actionAbout,            &QAction::triggered,       this, &MainWindow::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,          &QAction::triggered,       this, &MainWindow::on_actionAboutQt_triggered);
    connect(m_ui->actionAboutThisVersion, &QAction::triggered,       this, &MainWindow::aboutThisVersion);
    connect(m_ui->actionChangeTrigger,    &QAction::triggered,       m_ui->elementEditor, &ElementEditor::changeTriggerAction);
    connect(m_ui->actionDarkTheme,        &QAction::triggered,       this, &MainWindow::on_actionDarkTheme_triggered);
    connect(m_ui->actionEnglish,          &QAction::triggered,       this, &MainWindow::on_actionEnglish_triggered);
    connect(m_ui->actionExit,             &QAction::triggered,       this, &MainWindow::on_actionExit_triggered);
    connect(m_ui->actionExportToImage,    &QAction::triggered,       this, &MainWindow::on_actionExportToImage_triggered);
    connect(m_ui->actionExportToPdf,      &QAction::triggered,       this, &MainWindow::on_actionExportToPdf_triggered);
    connect(m_ui->actionFastMode,         &QAction::triggered,       this, &MainWindow::on_actionFastMode_triggered);
    connect(m_ui->actionFlipHorizontally, &QAction::triggered,       this, &MainWindow::on_actionFlipHorizontally_triggered);
    connect(m_ui->actionFlipVertically,   &QAction::triggered,       this, &MainWindow::on_actionFlipVertically_triggered);
    connect(m_ui->actionFullscreen,       &QAction::triggered,       this, &MainWindow::on_actionFullscreen_triggered);
    connect(m_ui->actionGates,            &QAction::triggered,       this, &MainWindow::on_actionGates_triggered);
    connect(m_ui->actionLabelsUnderIcons, &QAction::triggered,       this, &MainWindow::on_actionLabelsUnderIcons_triggered);
    connect(m_ui->actionLightTheme,       &QAction::triggered,       this, &MainWindow::on_actionLightTheme_triggered);
    connect(m_ui->actionMute,             &QAction::triggered,       this, &MainWindow::on_actionMute_triggered);
    connect(m_ui->actionNew,              &QAction::triggered,       this, &MainWindow::on_actionNew_triggered);
    connect(m_ui->actionOpen,             &QAction::triggered,       this, &MainWindow::on_actionOpen_triggered);
    connect(m_ui->actionPlay,             &QAction::triggered,       this, &MainWindow::on_actionPlay_triggered);
    connect(m_ui->actionPortuguese,       &QAction::triggered,       this, &MainWindow::on_actionPortuguese_triggered);
    connect(m_ui->actionReloadFile,       &QAction::triggered,       this, &MainWindow::on_actionReloadFile_triggered);
    connect(m_ui->actionRename,           &QAction::triggered,       m_ui->elementEditor, &ElementEditor::renameAction);
    connect(m_ui->actionResetZoom,        &QAction::triggered,       this, &MainWindow::on_actionResetZoom_triggered);
    connect(m_ui->actionRotateLeft,       &QAction::triggered,       this, &MainWindow::on_actionRotateLeft_triggered);
    connect(m_ui->actionRotateRight,      &QAction::triggered,       this, &MainWindow::on_actionRotateRight_triggered);
    connect(m_ui->actionSave,             &QAction::triggered,       this, &MainWindow::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,           &QAction::triggered,       this, &MainWindow::on_actionSaveAs_triggered);
    connect(m_ui->actionSelectAll,        &QAction::triggered,       this, &MainWindow::on_actionSelectAll_triggered);
    connect(m_ui->actionWaveform,         &QAction::triggered,       this, &MainWindow::on_actionWaveform_triggered);
    connect(m_ui->actionWires,            &QAction::triggered,       this, &MainWindow::on_actionWires_triggered);
    connect(m_ui->actionZoomIn,           &QAction::triggered,       this, &MainWindow::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,          &QAction::triggered,       this, &MainWindow::on_actionZoomOut_triggered);
    connect(m_ui->lineEdit,               &QLineEdit::returnPressed, this, &MainWindow::on_lineEdit_returnPressed);
    connect(m_ui->lineEdit,               &QLineEdit::textChanged,   this, &MainWindow::on_lineEdit_textChanged);

    qCDebug(zero) << "Window title.";
    setWindowTitle("WiRedPanda " + QString(APP_VERSION));

    qCDebug(zero) << "Building a new tab.";
    createNewWorkspace();

    qCDebug(zero) << "Opening file if not empty.";
    if (!fileName.isEmpty()) {
        loadPandaFile(fileName);
    }
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::loadAutosaveFiles(const QString &fileName)
{
    if (!Settings::contains("autosaveFile")) {
        qCDebug(zero) << "autosave files do no exist in config file.";
        return;
    }

    qCDebug(zero) << "autosave files exist in config file.";
    bool yesToAll = false;
    bool noToAll = false;
    QString allAutosaveFileNames(Settings::value("autosaveFile").toString());
    qCDebug(zero) << "all files:" << allAutosaveFileNames;
    QStringList autosaveFileNameList(allAutosaveFileNames.split("\t", SKIPEMPTYPARTS));
    for (const auto &autosaveFileName : qAsConst(autosaveFileNameList)) {
        if ((QFile(autosaveFileName).exists()) && (autosaveFileName != fileName)) {
            qCDebug(zero) << "Autosave exists and was not open. Trying to recover.";
            int selectedButton = QMessageBox::No;
            if ((!yesToAll) && (!noToAll)) {
                selectedButton = recoverAutosaveFile(autosaveFileName);
                if (selectedButton == QMessageBox::YesToAll) {
                    yesToAll = true;
                }
                if (selectedButton == QMessageBox::NoToAll) {
                    noToAll = true;
                }
            }
            if ((yesToAll) || (selectedButton == QMessageBox::Yes)) {
                if (!loadPandaFile(autosaveFileName)) {
                    selectedButton = autosaveFileDeleteAnyway(autosaveFileName);
                    if (selectedButton == QMessageBox::Yes) {
                        allAutosaveFileNames.remove(autosaveFileName + "\t");
                        Settings::setValue("autosaveFile", allAutosaveFileNames);
                    }
                }
            }
        } else if ((QFile(autosaveFileName).exists()) && (autosaveFileName == fileName)) {
            qCDebug(zero) << "Loading autosave.";
            m_loadedAutosave = true;
        } else if (!QFile(autosaveFileName).exists()) {
            qCDebug(zero) << "Removing autosave file name from config that does not exist.";
            allAutosaveFileNames.remove(autosaveFileName + "\t");
            Settings::setValue("autosaveFile", allAutosaveFileNames);
        }
    }
}

void MainWindow::createNewTab()
{
    qCDebug(zero) << "Adding tab. #tabs:" << m_ui->tab->count() << ", current tab:" << m_tabIndex;
    auto *workspace = new WorkSpace(this);
    workspace->view()->setFastMode(m_ui->actionFastMode->isChecked());
    workspace->scene()->updateTheme();
    m_ui->tab->addTab(workspace, "");
    qCDebug(zero) << "Finished #tabs:" << m_ui->tab->count() << ", current tab:" << m_tabIndex;
}

void MainWindow::createAutosaveFile()
{
    for (int i = 0; i < m_ui->tab->count(); ++i) {
        m_autosaveFile << new QTemporaryFile;
    }
}

void MainWindow::removeAutosaveFile(const int tabIndex)
{
    m_autosaveFile.takeAt(tabIndex);
}

void MainWindow::removeUndoRedoMenu()
{
    if (!m_currentTab) {
        return;
    }

    m_ui->menuEdit->removeAction(m_ui->menuEdit->actions().first());
    m_ui->menuEdit->removeAction(m_ui->menuEdit->actions().first());
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

bool MainWindow::save(QString fileName)
{
    qCDebug(zero) << "fileName:" << fileName;
    qCDebug(zero) << "Getting autosave settings info.";
    QString allAutosaveFileNames;
    if (Settings::contains("autosaveFile")) {
        allAutosaveFileNames = Settings::value("autosaveFile").toString();
    }
    qCDebug(zero) << "All auto save file names before save:" << allAutosaveFileNames;
    qCDebug(zero) << "Checking if it is an autosave file or new a new project, and ask for a fileName.";
    QString autosaveFileName;
    if ((fileName.isEmpty()) || ((!allAutosaveFileNames.isEmpty()) && (allAutosaveFileNames.contains(fileName)))) {
        qCDebug(zero) << "Should open window.";
        autosaveFileName = fileName;
        if (m_currentFile.fileName().isEmpty()) {
            fileName = QFileDialog::getSaveFileName(this, tr("Save File"), m_defaultDirectory, tr("Panda files (*.panda)"));
        }
    }
    if (fileName.isEmpty()) {
        return false;
    }
    if (!fileName.endsWith(".panda")) {
        fileName.append(".panda");
    }
    QSaveFile saveFile(fileName);
    if (!saveFile.open(QFile::WriteOnly)) {
        throw Pandaception(tr("Error opening file: ") + saveFile.errorString());
    }
    QDataStream stream(&saveFile);
    m_currentTab->save(stream, m_currentTab->dolphinFileName());
    if (!saveFile.commit()) {
        throw Pandaception(tr("Could not save file: ") + saveFile.errorString());
    }
    setCurrentFile(fileName);
    m_ui->statusBar->showMessage(tr("Saved file successfully."), 4000);
    m_currentTab->scene()->undoStack()->setClean();
    qCDebug(zero) << "Remove from autosave list recovered file that has been saved.";
    if (!autosaveFileName.isEmpty()) {
        allAutosaveFileNames.remove(autosaveFileName + "\t");
        Settings::setValue("autosaveFile", allAutosaveFileNames);
        qCDebug(zero) << "All auto save file names after removing recovered:" << allAutosaveFileNames;
    }
    qCDebug(zero) << "Remove autosave from settings and deleting it.";
    if (m_autosaveFile.at(m_tabIndex)->isOpen()) {
        allAutosaveFileNames.remove(m_autosaveFile.at(m_tabIndex)->fileName() + "\t");
        Settings::setValue("autosaveFile", allAutosaveFileNames + "\n");
        m_autosaveFile.at(m_tabIndex)->remove();
        qCDebug(zero) << "All auto save file names after removing autosave:" << allAutosaveFileNames;
    }
    return true;
}

void MainWindow::show()
{
    QMainWindow::show();
    if (!Settings::contains("showV4Warning")) {
        aboutThisVersion();
    }
}

void MainWindow::aboutThisVersion()
{
    qCDebug(zero) << "'showV4Warning' message box.";
    auto *checkBox = new QCheckBox(tr("Don't show this again."));
    checkBox->setChecked(Settings::contains("showV4Warning"));
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.setWindowTitle("WiRedPanda " + QString(APP_VERSION));
    msgBox.setText(
        tr("This version is not 100% compatible with previous versions of WiRedPanda.\n"
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
            Settings::setValue("showV4Warning", "true");
        } else {
            Settings::remove("showV4Warning");
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

int MainWindow::recoverAutosaveFile(const QString &autosaveFileName)
{
    if (!GlobalProperties::verbose) {
        return QMessageBox::NoToAll;
    }

    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);
    msgBox.setText(tr("An autosave file was found. Do you want to load it?\n") + autosaveFileName);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

int MainWindow::confirmSave(const bool multiple)
{
    // TODO: dont ask if current tab is empty
    QMessageBox msgBox;
    msgBox.setParent(this);
    if (multiple) {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
    } else {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    }
    QString fileName = m_currentFile.absoluteFilePath();
    if (fileName.isEmpty()) {
        fileName = tr("New Project");
    }
    msgBox.setText(fileName + tr(" has been modified. Do you want to save your changes?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

int MainWindow::autosaveFileDeleteAnyway(const QString &autosaveFileName)
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setText(tr("Error while opening autosave file. Do you want to delete it ?\n Autosave: ") + autosaveFileName);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}

void MainWindow::on_actionNew_triggered()
{
    createNewWorkspace();
}

void MainWindow::on_actionWires_triggered(const bool checked)
{
    m_currentTab->scene()->showWires(checked);
}

void MainWindow::on_actionRotateRight_triggered()
{
    m_currentTab->scene()->rotateRight();
}

void MainWindow::on_actionRotateLeft_triggered()
{
    m_currentTab->scene()->rotateLeft();
}

bool MainWindow::loadPandaFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        qCDebug(zero) << "Error: This file does not exist:" << fileName;
        return false;
    }
    qCDebug(zero) << "File exists";
    if (!file.open(QFile::ReadOnly)) {
        qCDebug(zero) << "Could not open file:" << file.errorString();
        return false;
    }
    qCDebug(zero) << "File opened:" << fileName;
    createNewWorkspace();
    qCDebug(zero) << "Current file set.";
    setCurrentFile(fileName);
    qCDebug(zero) << "Loading in editor.";
    QDataStream stream(&file);
    m_currentTab->load(stream);
    updateICList();
    qCDebug(zero) << "Closing file.";
    file.close();
    m_ui->statusBar->showMessage(tr("File loaded successfully."), 4000);
    return true;
}

void MainWindow::on_actionOpen_triggered()
{
    if (m_defaultDirectory.isEmpty()) {
        m_defaultDirectory = "./examples/";
    }

    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), m_defaultDirectory, tr("Panda files (*.panda)"));
    if (fileName.isEmpty()) {
        return;
    }
    loadPandaFile(fileName);
}

void MainWindow::on_actionSave_triggered()
{
    save(m_currentFile.absoluteFilePath());
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
    Settings::beginGroup("MainWindow");
    Settings::setValue("geometry", saveGeometry());
    Settings::setValue("windowState", saveState());
    Settings::beginGroup("splitter");
    Settings::setValue("geometry", m_ui->splitter->saveGeometry());
    Settings::setValue("state", m_ui->splitter->saveState());
    Settings::endGroup();
    Settings::endGroup();
}

bool MainWindow::hasModifiedFiles()
{
    QString allAutosaveFileNames;
    if (Settings::contains("autosaveFile")) {
        allAutosaveFileNames = Settings::value("autosaveFile").toString();
    }
    for (int i = 0; i < m_ui->tab->count(); ++i) {
        auto *workspace = static_cast<WorkSpace *>(m_ui->tab->widget(i));

        if (!workspace->scene()->undoStack()->isClean()) {
            return true;
        }

        const QString fileName = workspace->currentFile().fileName();

        if ((!fileName.isEmpty()) && (allAutosaveFileNames.contains(fileName))) {
            return true;
        }
    }
    return false;
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString path = m_defaultDirectory;
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

QFileInfo MainWindow::currentFile() const
{
    return m_currentFile;
}

QDir MainWindow::currentDir() const
{
    return m_currentFile.absoluteDir();
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    setAutosaveFileName(fileInfo);
    m_ui->tab->setTabText(m_tabIndex, fileName.isEmpty() ? tr("New Project") : fileName.split("/").last());
    m_currentFile = fileInfo;
    if (fileInfo.exists()) {
        updateICList();
    }
    if (m_currentTab) {
        m_currentTab->setCurrentFile(m_currentFile);
    }
    if (!m_loadedAutosave) {
        qCDebug(zero) << "Adding file to controller.";
        emit addRecentFile(fileInfo.absoluteFilePath());
    }
    qCDebug(zero) << "Setting global current file and dir.";
    GlobalProperties::currentFile = m_currentFile.absoluteFilePath();
    setCurrentDir();
}

void MainWindow::setAutosaveFileName(const QFileInfo &fileInfo)
{
    qCDebug(zero) << "Defining autosave path.";
    if (fileInfo.exists()) {
        QDir autosavePath(QDir::temp());
        qCDebug(zero) << "Autosave path set to the current file's directory, if there is one.";
        autosavePath.setPath(fileInfo.absolutePath());
        qCDebug(zero) << "Autosavepath:" << autosavePath.absolutePath();
        m_autosaveFile.at(m_tabIndex)->setFileTemplate(autosavePath.absoluteFilePath("." + fileInfo.baseName() + "XXXXXX.panda"));
        qCDebug(zero) << "Setting current file to:" << fileInfo.absoluteFilePath();
    } else {
        qCDebug(zero) << "Default file does not exist:" << fileInfo.absoluteFilePath();
        QDir autosavePath(QDir::temp());
        qCDebug(zero) << "Autosavepath:" << autosavePath.absolutePath();
        m_autosaveFile.at(m_tabIndex)->setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
        qCDebug(zero) << "Setting current file to random file in tmp.";
    }
}

void MainWindow::on_actionSelectAll_triggered()
{
    m_currentTab->scene()->selectAll();
}

void MainWindow::updateICList()
{
    m_ui->scrollAreaWidgetContents_Box->layout()->removeItem(m_ui->verticalSpacer_IC);
    for (auto *item : qAsConst(icItemWidgets)) {
        item->deleteLater();
    }
    icItemWidgets.clear();
    if (m_currentFile.exists()) {
        qCDebug(zero) << "Show files.";
        QDir directory(m_currentFile.absoluteDir());
        QStringList files = directory.entryList({"*.panda", "*.PANDA"}, QDir::Files);
        files.removeAll(m_currentFile.fileName());
        for (int i = files.size() - 1; i >= 0; --i) {
            if (files[i][0] == '.') {
                files.removeAt(i);
            }
        }
        qCDebug(zero) << "Files:" << files.join(", ");
        for (const QString &file : qAsConst(files)) {
            QPixmap pixmap(QString::fromUtf8(":/basic/box.png"));
            auto *item = new ListItemWidget(pixmap, ElementType::IC, file, this);
            icItemWidgets.append(item);
            m_ui->scrollAreaWidgetContents_Box->layout()->addWidget(item);
        }
    }
    m_ui->scrollAreaWidgetContents_Box->layout()->addItem(m_ui->verticalSpacer_IC);
}

bool MainWindow::closeTab(const int tabIndex)
{
    qCDebug(zero) << "Closing tab" << tabIndex << ", #tabs:" << m_ui->tab->count() << ", current tab:" << m_tabIndex;
    QString allAutosaveFileNames;
    QString autosaveFileName;
    qCDebug(zero) << "Reading autosave file names.";
    if (Settings::contains("autosaveFile")) {
        allAutosaveFileNames = Settings::value("autosaveFile").toString();
        qCDebug(zero) << "Verifying if this is a recovered autosave file.";
        if (allAutosaveFileNames.contains(m_currentTab->currentFile().fileName())) { // TODO: m_currentTab can be null here
            autosaveFileName = m_currentTab->currentFile().absoluteFilePath();
        }
    }
    qCDebug(zero) << "All auto save file names before closing tab:" << allAutosaveFileNames;
    qCDebug(zero) << "Checking if needs to save file.";
    if ((!m_currentTab->scene()->undoStack()->isClean()) || (!autosaveFileName.isEmpty())) { // TODO: m_currentTab can be null here
        selectTab(tabIndex);
        bool discardAutosaves = false;
        int selectedButton = confirmSave(false);
        if (selectedButton == QMessageBox::Cancel) {
            return false;
        }

        if (selectedButton == QMessageBox::Yes) {
            if (!save(m_currentTab->currentFile().absoluteFilePath())) {
                if (closeTabAnyway() == QMessageBox::No) {
                    return false;
                }

                discardAutosaves = true;
            }
        } else { // Close without saving. Deleting autosave if it was opened.
            discardAutosaves = true;
        }
        if (discardAutosaves) {
            if (!autosaveFileName.isEmpty()) {
                qCDebug(zero) << "Discarding recovered autosave file.";
                allAutosaveFileNames.remove(autosaveFileName + "\t");
                Settings::setValue("autosaveFile", allAutosaveFileNames);
                qCDebug(zero) << "All auto save file names after removing recovery file:" << allAutosaveFileNames;
            }
            if (!m_currentTab->scene()->undoStack()->isClean()) {
                qCDebug(zero) << "Discarding autosave modification.";
                allAutosaveFileNames.remove(m_autosaveFile.at(tabIndex)->fileName() + "\t");
                Settings::setValue("autosaveFile", allAutosaveFileNames);
                m_autosaveFile.at(tabIndex)->remove(); // This only removes the autosave file itself, not the QTempFile object.
                qCDebug(zero) << "All auto save file names after removing autosave file:" << allAutosaveFileNames;
            }
        }
    }
    qCDebug(zero) << "Moving to other tab if closed tab is the current one and not the only one. Othewise, if has just one tab, disconnects it from the UI.";
    if ((tabIndex == m_tabIndex) && (m_ui->tab->count() != 1)) {
        selectTab((tabIndex + 1) % m_ui->tab->count());
    } else if (m_ui->tab->count() == 1) {
        disconnectTab();
    }
    qCDebug(zero) << "Deleting tab and autosave.";
    removeAutosaveFile(tabIndex); // This removes the QTempFile object allocation.
    m_currentTab = nullptr;
    m_tabIndex = -1;
    m_ui->tab->removeTab(tabIndex);
    qCDebug(zero) << "Closed tab" << tabIndex << ", #tabs:" << m_ui->tab->count() << ", current tab:" << m_tabIndex;
    return true;
}

void MainWindow::setCurrentDir()
{
    if (m_currentFile.exists()) {
        m_defaultDirectory = m_currentFile.dir().absolutePath();
        m_ui->actionWaveform->setEnabled(true);
    } else {
        m_ui->actionWaveform->setDisabled(true);
    }
}

void MainWindow::disconnectTab()
{
    if (!m_currentTab) {
        return;
    }

    qCDebug(zero) << "Stopping simulation controller and event handling.";
    m_currentTab->scene()->setHandlingEvents(false);
    m_currentTab->simulationController()->stop();
    qCDebug(zero) << "Disconnecting zoom from UI controllers.";
    disconnect(m_currentTab->view(), &GraphicsView::zoomChanged, this, &MainWindow::zoomChanged);
    qCDebug(zero) << "Removing undo and redo actions from UI menu.";
    removeUndoRedoMenu();

    disconnect(m_currentTab->scene(), &Scene::circuitAppearenceHasChanged, this, &MainWindow::autosave);
    disconnect(m_currentTab->scene(), &Scene::circuitHasChanged, this, &MainWindow::autosave);
    disconnect(m_ui->actionClearSelection, &QAction::triggered, m_currentTab->scene(), &Scene::clearSelection);
    disconnect(m_ui->actionCopy, &QAction::triggered, m_currentTab->scene(), &Scene::copyAction);
    disconnect(m_ui->actionCut, &QAction::triggered, m_currentTab->scene(), &Scene::cutAction);
    disconnect(m_ui->actionDelete, &QAction::triggered, m_currentTab->scene(), &Scene::deleteAction);
    disconnect(m_ui->actionPaste, &QAction::triggered, m_currentTab->scene(), &Scene::pasteAction);
    disconnect(m_ui->elementEditor, &ElementEditor::sendCommand, m_currentTab->scene(), &Scene::receiveCommand);
}

void MainWindow::connectTab()
{
    qCDebug(zero) << "Setting editor elements to current tab: undo stack, scene, rectangle, simulation controller, IC manager. Also connecting IC Manager and simulation controller signals and setting global IC manager. Updating ICs if changed.";
    m_currentTab->selectWorkspace();
    qCDebug(zero) << "Connecting undo and redo functions to UI menu.";
    addUndoRedoMenu();
    qCDebug(zero) << "Setting Panda and Dolphin file info.";
    m_currentFile = m_currentTab->currentFile();
    GlobalProperties::currentFile = m_currentFile.absoluteFilePath();
    setCurrentDir();
    updateICList();
    qCDebug(zero) << "Connecting current tab to element editor menu in UI.";
    m_ui->elementEditor->setScene(m_currentTab->scene());

    connect(m_currentTab->scene(), &Scene::circuitAppearenceHasChanged, this, &MainWindow::autosave);
    connect(m_currentTab->scene(), &Scene::circuitHasChanged, this, &MainWindow::autosave);
    connect(m_currentTab->view(), &GraphicsView::zoomChanged, this, &MainWindow::zoomChanged);
    connect(m_ui->actionClearSelection, &QAction::triggered, m_currentTab->scene(), &Scene::clearSelection);
    connect(m_ui->actionCopy, &QAction::triggered, m_currentTab->scene(), &Scene::copyAction);
    connect(m_ui->actionCut, &QAction::triggered, m_currentTab->scene(), &Scene::cutAction);
    connect(m_ui->actionDelete, &QAction::triggered, m_currentTab->scene(), &Scene::deleteAction);
    connect(m_ui->actionPaste, &QAction::triggered, m_currentTab->scene(), &Scene::pasteAction);

    qCDebug(zero) << "Reinitialize simulation controller.";
    m_currentTab->simulationController()->start();
    m_currentTab->scene()->setHandlingEvents(true);
    m_currentTab->scene()->clearSelection();
    m_currentTab->view()->setFastMode(m_ui->actionFastMode->isChecked());
}

void MainWindow::createNewWorkspace()
{
    qCDebug(zero) << "Creating new workspace.";
    qCDebug(zero) << "Creating the tab structure.";
    createNewTab();
    qCDebug(zero) << "Creating autosave file.";
    createAutosaveFile();
    qCDebug(zero) << "Selecting the newly created tab.";
    m_ui->tab->setCurrentIndex(m_ui->tab->count() - 1);
    qCDebug(zero) << "Setting directories and autosave file.";
    setCurrentFile();
}

void MainWindow::selectTab(const int tabIndex)
{
    if (tabIndex == -1) {
        return;
    }

    if (m_currentTab) {
        disconnectTab(); // disconnect previously selected tab
    }

    m_currentTab = static_cast<WorkSpace *>(m_ui->tab->currentWidget());
    m_tabIndex = m_ui->tab->currentIndex();
    qCDebug(zero) << "Selecting tab:" << tabIndex;
    connectTab();

    qCDebug(zero) << "New tab selected. BD fileName:" << m_currentTab->dolphinFileName();
}

void MainWindow::on_lineEdit_textChanged(const QString &text)
{
    m_ui->searchLayout->removeItem(m_ui->VSpacer);
    for (auto *item : qAsConst(searchItemWidgets)) {
        item->deleteLater();
    }
    searchItemWidgets.clear();
    m_firstResult = nullptr;
    if (text.isEmpty()) {
        m_ui->searchScrollArea->hide();
        m_ui->tabElements->show();
    } else {
        m_ui->searchScrollArea->show();
        m_ui->tabElements->hide();
        auto ics = m_ui->tabElements->findChildren<Label *>("label_ic");
        QRegularExpression regex(QString(".*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        QRegularExpression regex2(QString("^label_.*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        auto searchResults = m_ui->tabElements->findChildren<Label *>(regex2);
        auto allLabels = m_ui->tabElements->findChildren<Label *>();
        for (auto *label : qAsConst(allLabels)) {
            if (regex.match(label->name()).hasMatch()) {
                if (!searchResults.contains(label)) {
                    searchResults.append(label);
                }
            }
        }
        for (auto *ic : qAsConst(ics)) {
            if (regex.match(ic->auxData()).hasMatch()) {
                searchResults.append(ic);
            }
        }
        for (auto *label : qAsConst(searchResults)) {
            auto *item = new ListItemWidget(label->pixmap(), label->elementType(), label->auxData());
            if (!m_firstResult) {
                m_firstResult = item->label();
            }
            m_ui->searchLayout->addWidget(item);
            searchItemWidgets.append(item);
        }
    }
    m_ui->searchLayout->addItem(m_ui->VSpacer);
}

void MainWindow::on_lineEdit_returnPressed()
{
    if (m_firstResult) {
        m_firstResult->startDrag(); // TODO: instead of drag use insert item at mouse position?
        m_ui->lineEdit->clear();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    if (m_currentTab) {
        auto *scene = m_currentTab->scene();
        auto *view = m_currentTab->view();
        scene->setSceneRect(scene->sceneRect().united(view->rect()));
    }
}

void MainWindow::on_actionReloadFile_triggered()
{
    if (!m_currentFile.exists()) {
        return;
    }

    auto *undostack = m_currentTab->scene()->undoStack();
    if (!undostack->isClean()) {
        const int selectedButton = confirmSave(false);
        if (selectedButton == QMessageBox::Cancel) {
            return;
        }

        if (selectedButton == QMessageBox::Yes) {
            if (!save(m_currentTab->currentFile().absoluteFilePath())) {
                if (closeTabAnyway() == QMessageBox::No) {
                    return;
                }
            }
        }
        m_currentTab->scene()->selectAll();
        m_currentTab->scene()->deleteAction();
        m_currentTab->scene()->undoStack()->clear();
        loadPandaFile(m_currentFile.absoluteFilePath());
    }
}

void MainWindow::on_actionGates_triggered(const bool checked)
{
    m_ui->actionWires->setEnabled(checked);
    m_currentTab->scene()->showWires(checked ? m_ui->actionWires->isChecked() : checked);
    m_currentTab->scene()->showGates(checked);
}

void MainWindow::exportToArduino(QString fileName)
{
    if (fileName.isEmpty()) {
        throw Pandaception("Missing file name.");
    }
    auto elements = m_currentTab->scene()->elements();
    if (elements.isEmpty()) {
        throw Pandaception("The panda file is empty.");
    }
    auto *simController = m_currentTab->simulationController();
    simController->stop();
    if (!fileName.endsWith(".ino")) {
        fileName.append(".ino");
    }
    elements = ElementMapping::sortGraphicElements(elements);

    // TODO: why home()? why not the current folder? what if the user type a path?
    CodeGenerator arduino(QDir::home().absoluteFilePath(fileName), elements);
    arduino.generate();
    simController->start();
    m_ui->statusBar->showMessage(tr("Arduino code successfully generated."), 4000);

    qCDebug(zero) << "Arduino code successfully generated.";
}

void MainWindow::exportToWaveFormFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        throw Pandaception("Missing file name.");
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
    const QString fileName = QFileDialog::getSaveFileName(this, tr("Generate Arduino Code"), m_defaultDirectory, tr("Arduino file (*.ino)"));

    if (!fileName.isEmpty()) {
        exportToArduino(fileName);
    }
}

void MainWindow::on_actionZoomIn_triggered() const
{
    if (m_currentTab) {
        m_currentTab->view()->zoomIn();
    }
}

void MainWindow::on_actionZoomOut_triggered() const
{
    if (m_currentTab) {
        m_currentTab->view()->zoomOut();
    }
}

void MainWindow::on_actionResetZoom_triggered() const
{
    if (m_currentTab) {
        m_currentTab->view()->resetZoom();
    }
}

void MainWindow::zoomChanged()
{
    m_ui->actionZoomIn->setEnabled(m_currentTab->view()->canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_currentTab->view()->canZoomOut());
}

void MainWindow::updateRecentFileActions()
{
    const auto files = m_recentFilesController->recentFiles();
    const int numRecentFiles = qMin(files.size(), RecentFilesController::MaxRecentFiles);
    if (numRecentFiles > 0) {
        m_ui->menuRecent_files->setEnabled(true);
    }
    for (int i = 0; i < numRecentFiles; ++i) {
        const QString text = "&" + QString::number(i + 1) + " " + QFileInfo(files.at(i)).fileName();
        m_recentFilesActions.at(i)->setText(text);
        m_recentFilesActions.at(i)->setData(files[i]);
        m_recentFilesActions.at(i)->setVisible(true);
    }
    for (int i = numRecentFiles; i < RecentFilesController::MaxRecentFiles; ++i) {
        m_recentFilesActions.at(i)->setVisible(false);
    }
}

void MainWindow::openRecentFile()
{
    auto *action = qobject_cast<QAction *>(sender());

    if (action) {
        loadPandaFile(action->data().toString());
    }
}

void MainWindow::createRecentFileActions()
{
    for (int i = 0; i < RecentFilesController::MaxRecentFiles; ++i) {
        auto *action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
        m_ui->menuRecent_files->addAction(action);
        m_recentFilesActions << action;
    }
    updateRecentFileActions();
}

void MainWindow::on_actionExportToPdf_triggered()
{
    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), m_defaultDirectory, tr("PDF files (*.pdf)"));
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
}

void MainWindow::on_actionExportToImage_triggered()
{
    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), m_defaultDirectory, tr("PNG files (*.png)"));
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
}

void MainWindow::retranslateUi()
{
    m_ui->retranslateUi(this);
    m_ui->elementEditor->retranslateUi();

    auto items = m_ui->tabElements->findChildren<ListItemWidget *>();
    for (auto *item : qAsConst(items)) {
        item->updateName();
    }
}

void MainWindow::loadTranslation(const QString &language)
{
    if (language.isEmpty()) {
        return;
    }

    qApp->removeTranslator(m_pandaTranslator);
    qApp->removeTranslator(m_qtTranslator);

    QString pandaFile;
    QString qtFile;

    if (language == "://wpanda_pt_BR.qm") {
        pandaFile = "://wpanda_pt_BR.qm";
        qtFile = "://qt_pt_BR.qm";
    }

    if (language == "://wpanda_en.qm") {
        return retranslateUi();
    }

    if (pandaFile.isEmpty() || qtFile.isEmpty()) {
        return;
    }

    m_pandaTranslator = new QTranslator(this);

    if (!m_pandaTranslator->load(pandaFile) || !qApp->installTranslator(m_pandaTranslator)) {
        throw Pandaception("Error loading WiRedPanda translation!");
    }

    m_qtTranslator = new QTranslator(this);

    if (!m_qtTranslator->load(qtFile) || !qApp->installTranslator(m_qtTranslator)) {
        throw Pandaception("Error loading Qt translation!");
    }

    retranslateUi();
}

void MainWindow::on_actionEnglish_triggered()
{
    const QString language = "://wpanda_en.qm";
    Settings::setValue("language", language);
    loadTranslation(language);
}

void MainWindow::on_actionPortuguese_triggered()
{
    const QString language = "://wpanda_pt_BR.qm";
    Settings::setValue("language", language);
    loadTranslation(language);
}

void MainWindow::on_actionPlay_triggered(const bool checked)
{
    if (!m_currentTab) {
        return;
    }

    auto *controller = m_currentTab->simulationController();

    checked ? controller->start() : controller->stop();
    controller->updateAll();
}

void MainWindow::populateMenu(QSpacerItem *spacer, const QStringList &names, QLayout *layout) // TODO: call this function with the list of files and set directory watcher.
{
    layout->removeItem(spacer);

    for (const auto &name : names) {
        auto type = ElementFactory::textToType(name);
        auto pixmap(ElementFactory::pixmap(type));
        layout->addWidget(new ListItemWidget(pixmap, type, name, this));
    }

    layout->addItem(spacer);
}

void MainWindow::populateLeftMenu()
{
    m_ui->tabElements->setCurrentIndex(0);
    populateMenu(m_ui->verticalSpacer_InOut, {"InputVcc", "InputGnd", "InputButton", "InputSwitch", "InputRotary", "Clock", "Led", "Display", "Display14", "Buzzer"}, m_ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(m_ui->verticalSpacer_Gates, {"And", "Or", "Not", "Nand", "Nor", "Xor", "Xnor", "Mux", "Demux", "Node"}, m_ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(m_ui->verticalSpacer_Memory, {"DFlipFlop", "DLatch", "JKFlipFlop", "SRFlipFlop", "TFlipFlop"}, m_ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(m_ui->verticalSpacer_MISC, {"Text", "Line"}, m_ui->scrollAreaWidgetContents_Misc->layout());
}

void MainWindow::on_actionFastMode_triggered(const bool checked)
{
    setFastMode(checked);
    Settings::setValue("fastMode", checked);
}

void MainWindow::on_actionWaveform_triggered()
{
    auto *bewavedDolphin = new BewavedDolphin(m_currentTab->scene(), true, this);
    bewavedDolphin->createWaveform(m_currentTab->dolphinFileName());
    qCDebug(zero) << "BD fileName:" << m_currentTab->dolphinFileName();
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
}

void MainWindow::on_actionFlipHorizontally_triggered()
{
    m_currentTab->scene()->flipHorizontally();
}

void MainWindow::on_actionFlipVertically_triggered()
{
    m_currentTab->scene()->flipVertically();
}

QString MainWindow::dolphinFileName()
{
    return m_currentTab->dolphinFileName();
}

void MainWindow::setDolphinFileName(const QString &fileName)
{
    m_currentTab->setDolphinFileName(fileName);
}

void MainWindow::on_actionFullscreen_triggered()
{
    isFullScreen() ? showNormal() : showFullScreen();
}

void MainWindow::autosave()
{
    qCDebug(two) << "Starting autosave.";
    QString allAutosaveFileNames;
    qCDebug(zero) << "Cheking if autosavefile exists and if it contains current project file. If so, remove autosavefile from it.";
    if (Settings::contains("autosaveFile")) {
        allAutosaveFileNames = Settings::value("autosaveFile").toString();
        qCDebug(three) << "All auto save file names before autosaving:" << allAutosaveFileNames;
        if ((!m_autosaveFile.at(m_tabIndex)->fileName().isEmpty()) && (allAutosaveFileNames.contains(m_autosaveFile.at(m_tabIndex)->fileName()))) {
            qCDebug(three) << "Removing current autosave file name.";
            allAutosaveFileNames.remove(m_autosaveFile.at(m_tabIndex)->fileName() + "\t");
            Settings::setValue("autosaveFile", allAutosaveFileNames);
        }
    }
    qCDebug(zero) << "All auto save file names after possibly removing autosave:" << allAutosaveFileNames;
    qCDebug(zero) << "If autosave exists and undo stack is clean, remove it.";
    auto *undostack = m_currentTab->scene()->undoStack();
    qCDebug(zero) << "undostack element:" << undostack->index() << "of" << undostack->count();
    if (undostack->isClean()) {
        m_ui->tab->setTabText(m_tabIndex, m_currentFile.exists() ? m_currentFile.fileName() : tr("New Project"));
        qCDebug(three) << "Undo stack is clean.";
        if (m_autosaveFile.at(m_tabIndex)->exists()) {
            m_autosaveFile.at(m_tabIndex)->remove();
        }
    } else {
        qCDebug(three) << "Undo is !clean. Must set autosave file.";
        m_ui->tab->setTabText(m_tabIndex, m_currentFile.exists() ? m_currentFile.fileName() + "*" : tr("New Project*"));
        if (m_autosaveFile.at(m_tabIndex)->exists()) {
            qCDebug(three) << "Autosave file already exists. Delete it to update.";
            m_autosaveFile.at(m_tabIndex)->remove();
            if (!m_currentFile.fileName().isEmpty()) {
                QDir autosavePath(QDir::temp());
                qCDebug(three) << "Autosave path set to the current file's directory, if there is one.";
                autosavePath.setPath(m_currentFile.absolutePath());
                qCDebug(three) << "Autosavepath:" << autosavePath.absolutePath();
                m_autosaveFile.at(m_tabIndex)->setFileTemplate(autosavePath.absoluteFilePath("." + m_currentFile.baseName() + "XXXXXX.panda"));
                qCDebug(three) << "Setting current file to:" << m_currentFile.absoluteFilePath();
            } else {
                qCDebug(three) << "Default value not set yet.";
                QDir autosavePath(QDir::temp());
                qCDebug(three) << "Autosavepath:" << autosavePath.absolutePath();
                m_autosaveFile.at(m_tabIndex)->setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
                qCDebug(three) << "Setting current file to random file in tmp.";
            }
        }
        if (m_autosaveFile.at(m_tabIndex)->open()) {
            qCDebug(three) << "Writing to autosave file.";
            QDataStream stream(m_autosaveFile.at(m_tabIndex));
            QString autosaveFileName = m_autosaveFile.at(m_tabIndex)->fileName() + "\t";
            try {
                m_currentTab->save(stream, m_currentTab->dolphinFileName());
                allAutosaveFileNames.append(autosaveFileName);
                Settings::setValue("autosaveFile", allAutosaveFileNames);
                qCDebug(three) << "All auto save file names after adding autosave:" << allAutosaveFileNames;
            } catch (std::runtime_error &e) {
                qCDebug(zero) << "Error autosaving project:" << e.what();
                m_autosaveFile.at(m_tabIndex)->close();
                m_autosaveFile.at(m_tabIndex)->remove();
            }
            m_autosaveFile.at(m_tabIndex)->close();
        }
    }
    qCDebug(three) << "Finished autosave.";
}

void MainWindow::on_actionMute_triggered(const bool checked)
{
    m_currentTab->scene()->mute(checked);
    m_ui->actionMute->setText(checked ? tr("Unmute") : tr("Mute"));
}

void MainWindow::on_actionLabelsUnderIcons_triggered(const bool checked)
{
    m_ui->mainToolBar->setToolButtonStyle(checked ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::WindowActivate:
        if (m_ui->actionPlay->isChecked()) {
            on_actionPlay_triggered(true);
        }
        break;

    case QEvent::WindowDeactivate:
            on_actionPlay_triggered(false);
        break;

    default: break;
    };

    return QMainWindow::event(event);
}
