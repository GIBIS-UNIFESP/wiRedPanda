// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include <QCloseEvent>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QProcess>
#include <QRectF>
#include <QSaveFile>
#include <QSettings>
#include <QShortcut>
#include <QSpacerItem>
#include <QTranslator>
#include <QUndoStack>

#include "arduino/codegenerator.h"
#include "bewaveddolphin.h"
#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicsview.h"
#include "graphicsviewzoom.h"
#include "label.h"
#include "listitemwidget.h"
#include "thememanager.h"
#include "simulationcontroller.h"
#include "ui_mainwindow.h"
#include "workspace.h"

MainWindow::MainWindow(QWidget *parent, const QString &filename)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_editor(new Editor(this))
    , m_firstResult(nullptr)
    , m_loadedAutoSave(false)
    , m_dolphinFileName("")
    , m_bd(nullptr)
    , m_autoSaveFile(nullptr)
    , m_translator(nullptr)
{
    COMMENT("WIRED PANDA Version = " << APP_VERSION << " OR " << GlobalProperties::version, 0);
    ui->setupUi(this);
    ThemeManager::globalMngr = new ThemeManager(this);
    setWindowTitle("wiRED PANDA v" + QString(APP_VERSION));
    /* Translation */
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    COMMENT("Settings filename: " << settings.fileName().toStdString(), 0);
    if (settings.value("language").isValid()) {
        loadTranslation(settings.value("language").toString());
    }

    COMMENT("Building dialog within a new tab.", 0);
    m_current_tab = 0;
    createNewWorkspace(tr("New Project"));
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabBarClicked, this, &MainWindow::selectTab);
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    COMMENT("Restoring geometry and setting zoom controls.", 0);
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    settings.beginGroup("splitter");
    ui->splitter->restoreGeometry(settings.value("geometry").toByteArray());
    ui->splitter->restoreState(settings.value("state").toByteArray());
    ui->actionExport_to_Arduino->setEnabled(false);
    settings.endGroup();
    settings.endGroup();
    QList<QKeySequence> zoom_in_shortcuts;
    zoom_in_shortcuts << QKeySequence("Ctrl++") << QKeySequence("Ctrl+=");
    ui->actionZoom_in->setShortcuts(zoom_in_shortcuts);

    COMMENT("Preparing theme and UI modes.",0);
    auto *themeGroup = new QActionGroup(this);
    auto const actions = ui->menuTheme->actions();
    for (QAction *action : actions) {
        themeGroup->addAction(action);
    }
    themeGroup->setExclusive(true);
    connect(ThemeManager::globalMngr, &ThemeManager::themeChanged, this, &MainWindow::updateTheme);
    connect(ThemeManager::globalMngr, &ThemeManager::themeChanged, m_editor, &Editor::updateTheme);
    ThemeManager::globalMngr->initialize();
    if (settings.value("fastMode").isValid()) {
        setFastMode(settings.value("fastMode").toBool());
    } else {
        setFastMode(false);
    }

    COMMENT("Setting left side menus.", 0);
    m_editor->setElementEditor(ui->widgetElementEditor);
    ui->searchScrollArea->hide();
    auto *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this);
    connect(shortcut, &QShortcut::activated, ui->lineEdit, QOverload<>::of(&QWidget::setFocus));
    populateLeftMenu();

    COMMENT("Setting directories and autosave file.", 0);
    setCurrentFile(QFileInfo());

    COMMENT("Creating edit menu funcionalities.", 0);
    connect(ui->actionCopy, &QAction::triggered, m_editor, &Editor::copyAction);
    connect(ui->actionCut, &QAction::triggered, m_editor, &Editor::cutAction);
    connect(ui->actionPaste, &QAction::triggered, m_editor, &Editor::pasteAction);
    connect(ui->actionDelete, &QAction::triggered, m_editor, &Editor::deleteAction);

    COMMENT("Setting up zoom and screen funcionalities.", 0);
    connect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
    connect(m_editor, &Editor::scroll, this, &MainWindow::scrollView);
    connect(m_editor, &Editor::circuitHasChanged, this, &MainWindow::autoSave);
    m_fullscreenView->setCacheMode(QGraphicsView::CacheBackground);

    COMMENT("Loading recent file list.", 0);
    m_rfController = new RecentFilesController("recentFileList", this, true);
    m_ricController = new RecentFilesController("recentICs", this, false);
    connect(this, &MainWindow::addRecentFile, m_rfController, &RecentFilesController::addRecentFile);
    connect(this, &MainWindow::addRecentIcFile, m_ricController, &RecentFilesController::addRecentFile);
    m_firstResult = nullptr;
    updateRecentICs();
    createRecentFileActions();
    connect(m_rfController, &RecentFilesController::recentFilesUpdated, this, &MainWindow::updateRecentFileActions);
    connect(m_ricController, &RecentFilesController::recentFilesUpdated, this, &MainWindow::updateRecentICs);

    COMMENT("Checking for autosave file recovery.", 0);
    loadAutoSaveFiles(settings, filename);
    COMMENT("Checking playing simulation.", 0);
    ui->actionPlay->setChecked(true);
}

void MainWindow::loadAutoSaveFiles(QSettings &settings, const QString &filename)
{
    if (settings.contains("autosaveFile")) {
        COMMENT("autosave files exist in config file.", 0);
        bool yes_to_all = false;
        bool no_to_all = false;
        QString allAutoSaveFileName(settings.value("autosaveFile").toString());
        QStringList autoSaveFileNameList(allAutoSaveFileName.split("\t",Qt::SkipEmptyParts));
        COMMENT("all files: " << allAutoSaveFileName.toStdString(), 0);
        foreach(auto autoSaveFileName, autoSaveFileNameList) {
            if ((QFile(autoSaveFileName).exists()) && (autoSaveFileName != filename)) {
                COMMENT("Autosave exists and was not open. Trying to recover.", 0);
                int ret = QMessageBox::No;
                if ((!yes_to_all)&&(!no_to_all)) {
                    ret = recoverAutoSaveFile(autoSaveFileName);
                    if (ret == QMessageBox::YesToAll) {
                        yes_to_all = true;
                    }
                    if (ret == QMessageBox::NoToAll) {
                        no_to_all = true;
                    }
                }
                if ((yes_to_all)||(ret == QMessageBox::Yes)) {
                    if (!loadPandaFile(autoSaveFileName)) {
                        ret = autoSaveFileDeleteAnyway(autoSaveFileName);
                        if (ret == QMessageBox::Yes) {
                            allAutoSaveFileName.remove(autoSaveFileName+"\t");
                            settings.setValue("autosaveFile", allAutoSaveFileName);
                        }
                    }
                }
            } else if ((QFile(autoSaveFileName).exists()) && (autoSaveFileName == filename)) {
                COMMENT("Loading autosave!", 0);
                m_loadedAutoSave = true;
            } else if (!QFile(autoSaveFileName).exists()) {
                COMMENT("Removing autosave file name from config that does not exist.", 0);
                allAutoSaveFileName.remove(autoSaveFileName+"\t");
                settings.setValue("autosaveFile", allAutoSaveFileName);
            }
        }
    } else {
        COMMENT("autosave files do no exist in config file.", 0);
    }
}

void MainWindow::createNewTab(const QString &tab_name) {
    COMMENT("Dialog built. Now adding tab. #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
    auto new_tab = new QWidget(ui->tabWidget_mainWindow);
    new_tab->setLayout(m_fullscreenDlg->layout());
    new_tab->layout()->addWidget(m_fullscreenView);
    ui->tabWidget_mainWindow->addTab(new_tab, tab_name);
    COMMENT("Storing tab pointers.", 0 );
    m_tabs.push_back(WorkSpace(m_fullscreenDlg, m_fullscreenView, m_editor));
    COMMENT("Files.", 0);
    addAutosaveFile();
    COMMENT("Setting scene.", 0);
    m_fullscreenView->setScene(m_editor->getScene());
    COMMENT("Finished #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
}

void MainWindow::addAutosaveFile()
{
    QTemporaryFile** new_vector = new QTemporaryFile*[m_tabs.size()];
    for (int idx = 0; idx < m_tabs.size() - 1; ++idx) {
        new_vector[idx] = m_autoSaveFile[idx];
    }
    new_vector[m_tabs.size() - 1] = new QTemporaryFile();
    delete []m_autoSaveFile;
    m_autoSaveFile = new_vector;
}

void MainWindow::removeAutosaveFile(int tab)
{
    QTemporaryFile** new_vector = nullptr;
    if (m_tabs.size() > 1) {
        new_vector = new QTemporaryFile*[m_tabs.size()-1];
        for (int idx = 0; idx < tab; ++idx) {
            new_vector[idx] = m_autoSaveFile[idx];
        }
        for (int idx = tab+1; idx < m_tabs.size(); ++idx) {
            new_vector[idx-1] = m_autoSaveFile[idx];
        }
    }
    delete m_autoSaveFile[tab];
    delete []m_autoSaveFile;
    m_autoSaveFile = new_vector;
}

void MainWindow::createUndoRedoMenus()
{
    m_undoAction.push_back(m_editor->getUndoStack()->createUndoAction(this, tr("&Undo")));
    m_undoAction[m_undoAction.size()-1]->setIcon(QIcon(QPixmap(":/toolbar/undo.png")));
    m_undoAction[m_undoAction.size()-1]->setShortcuts(QKeySequence::Undo);
    m_redoAction.push_back(m_editor->getUndoStack()->createRedoAction(this, tr("&Redo")));
    m_redoAction[m_undoAction.size()-1]->setIcon(QIcon(QPixmap(":/toolbar/redo.png")));
    m_redoAction[m_undoAction.size()-1]->setShortcuts(QKeySequence::Redo);
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), m_undoAction[m_undoAction.size()-1]);
    ui->menuEdit->insertAction(m_undoAction[m_undoAction.size()-1], m_redoAction[m_undoAction.size()-1]);
}

void MainWindow::selectUndoRedoMenu(int tab)
{
    ui->menuEdit->removeAction(m_undoAction[m_current_tab]);
    ui->menuEdit->removeAction(m_redoAction[m_current_tab]);
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), m_undoAction[tab]);
    ui->menuEdit->insertAction(m_undoAction[tab], m_redoAction[tab]);
}

void MainWindow::setFastMode(bool fastModeEnabled)
{
    m_fullscreenView->setRenderHint(QPainter::Antialiasing, !fastModeEnabled);
    m_fullscreenView->setRenderHint(QPainter::SmoothPixmapTransform, !fastModeEnabled);
    ui->actionFast_Mode->setChecked(fastModeEnabled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

bool MainWindow::save(QString fname)
{
    COMMENT("fname: " << fname.toStdString(), 0);
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    QString allAutoSaveFileName;
    if (settings.contains("autosaveFile")) {
        allAutoSaveFileName = settings.value("autosaveFile").toString();
    }
    COMMENT("All auto save file names before save: " << allAutoSaveFileName.toStdString(), 0);
    COMMENT("Checking if it is an autosave file or new a new project, and ask for a filename.", 0);
    QString autoSaveFileName;
    if ((fname.isEmpty()) || ((!allAutoSaveFileName.isEmpty()) && (allAutoSaveFileName.contains(fname)))) {
        COMMENT("Should open window!", 0);
        autoSaveFileName = fname;
        if (m_currentFile.fileName().isEmpty()) {
            fname = QFileDialog::getSaveFileName(this, tr("Save File"), m_defaultDirectory.absolutePath(), tr("Panda files (*.panda)"));
        }
    }
    if (fname.isEmpty()) {
        return false;
    }
    if (!fname.endsWith(".panda")) {
        fname.append(".panda");
    }
    QSaveFile fl(fname);
    if (fl.open(QFile::WriteOnly)) {
        QDataStream ds(&fl);
        try {
            m_editor->save(ds, m_dolphinFileName);
        } catch (std::runtime_error &e) {
            std::cerr << tr("Error saving project: ").toStdString() << e.what() << std::endl;
            return false;
        }
    }
    if (fl.commit()) {
        setCurrentFile(QFileInfo(fname));
        ui->statusBar->showMessage(tr("Saved file successfully."), 2000);
        m_editor->getUndoStack()->setClean();
        COMMENT("Remove from autosave list recovered file that has been saved.", 0)
        if (!autoSaveFileName.isEmpty()) {
            allAutoSaveFileName.remove(autoSaveFileName+"\t");
            settings.setValue("autosaveFile", allAutoSaveFileName);
            COMMENT("All auto save file names after removing recovered: " << allAutoSaveFileName.toStdString(), 0);
        }
        COMMENT("Remove autosave from settings and deleting it.", 0);
        if (m_autoSaveFile[m_current_tab]->isOpen()) {
            allAutoSaveFileName.remove(m_autoSaveFile[m_current_tab]->fileName());
            settings.setValue("autosaveFile", allAutoSaveFileName+"\n");
            m_autoSaveFile[m_current_tab]->remove();
            COMMENT("All auto save file names after removing autosave: " << allAutoSaveFileName.toStdString(), 0);
        }
        return true;
    }
    std::cerr << QString(tr("Could not save file: ") + fl.errorString() + ".").toStdString() << std::endl;
    return false;
}

void MainWindow::show()
{
    QMainWindow::show();
}

int MainWindow::closeTabAnyway()
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(QString(tr("File not saved. Close tab anyway?")));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}


int MainWindow::recoverAutoSaveFile(const QString& autosaveFilename)
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);
    msgBox.setText(QString(tr("We have found an autosave file. Do you want to load it?\n Autosave: ") + autosaveFilename));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

int MainWindow::confirmSave(bool multiple)
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    if (multiple) {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
    } else {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    }
    msgBox.setText(tr("Do you want to save your changes?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

int MainWindow::autoSaveFileDeleteAnyway(const QString& autosaveFilename)
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setText(QString(tr("Error while opening autosave file. Do you want to delete it ?\n Autosave: ") + autosaveFilename));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}

void MainWindow::createNewWorkspace(const QString &fname) // Use this function always... Call it from MainWindow constructor.
{
    COMMENT("Creating new workspace: " << fname.toStdString(), 0);
    m_editor->setupWorkspace();
    buildFullScreenDialog();
    createNewTab(fname);
    createUndoRedoMenus();
    emit ui->tabWidget_mainWindow->tabBarClicked(m_tabs.size()-1); // Move these to on_actionNew_triggered.
    ui->tabWidget_mainWindow->setCurrentIndex(m_tabs.size()-1);
    COMMENT("Finished creating new workspace: " << fname.toStdString(), 0);
}


void MainWindow::on_actionNew_triggered()
{
    createNewWorkspace(tr("New Project"));
}

void MainWindow::on_actionWires_triggered(bool checked)
{
    m_editor->showWires(checked);
}

void MainWindow::on_actionRotate_right_triggered()
{
    m_editor->rotate(true);
}

void MainWindow::on_actionRotate_left_triggered()
{
    m_editor->rotate(false);
}

bool MainWindow::loadPandaFile(const QString &fname)
{
    QFile fl(fname);
    if (!fl.exists()) {
        QMessageBox::warning(this, tr("Error!"), tr("File \"%1\" does not exist!").arg(fname), QMessageBox::Ok, QMessageBox::NoButton);
        std::cerr << tr("Error: This file does not exist: ").toStdString() << fname.toStdString() << std::endl;
        COMMENT("Here!", 0);
        return false;
    }
    COMMENT("File exists.", 0);
    if (fl.open(QFile::ReadOnly)) {
        COMMENT("File opened: " << fname.toStdString(), 0);
        QDataStream ds(&fl);
        int current_tab = m_current_tab;
        createNewWorkspace(QFileInfo(fl).fileName());
        try {
            COMMENT("Loading in editor.", 0);
            GlobalProperties::currentFile = fname; // Do this to search for IC files.
            m_editor->load(ds, fname);
            COMMENT("Current file set.", 0);
            setCurrentFile(QFileInfo(fname));
            COMMENT("Loaded. Emitting changed signal.", 0);
            emit m_editor->circuitHasChanged();
            COMMENT("Finished updating changed by signal.", 0);
        } catch (std::runtime_error &e) {
            std::cerr << tr("Error loading project: ").toStdString() << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error!"), tr("Could not open file.\nError: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::NoButton);
            closeTab(m_tabs.size()-1);
            emit ui->tabWidget_mainWindow->tabBarClicked(current_tab);
            return false;
        }
    } else {
        std::cerr << tr("Could not open file in ReadOnly mode : ").toStdString() << fname.toStdString() << "." << std::endl;
        return false;
    }
    COMMENT("Closing file.", 0);
    fl.close();
    ui->statusBar->showMessage(tr("File loaded successfully."), 2000);
    return true;
}

void MainWindow::scrollView(int dx, int dy)
{
    m_fullscreenView->scroll(dx, dy);
}

void MainWindow::on_actionOpen_triggered()
{
    QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), m_defaultDirectory.absolutePath(), tr("Panda files (*.panda)"));
    if (fname.isEmpty()) {
        return;
    }
    loadPandaFile(fname);
}

void MainWindow::on_actionSave_triggered()
{
    save(m_currentFile.absoluteFilePath());
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       "wiRED Panda",
                       tr("<p>wiRED Panda is a software developed by the students of the Federal University of São Paulo."
                          " This project was created in order to help students to learn about logic circuits.</p>"
                          "<p>Software version: %1</p>"
                          "<p><strong>Creators:</strong></p>"
                          "<ul>"
                          "<li> Davi Morales </li>"
                          "<li> Lucas Lellis </li>"
                          "<li> Rodrigo Torres </li>"
                          "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
                          "</ul>"
                          "<p> wiRed Panda is currently maintained by Prof. Fábio Cappabianco, Ph.D. and Vinícius R. Miguel.</p>"
                          "<p> Please file a report at our GitHub page if bugs are found or if you wish for a new functionality to be implemented.</p>"
                          "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>")
                           .arg(QApplication::applicationVersion()));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

bool MainWindow::closeFile()
{
    while(m_tabs.size() > 0) {
        int tab_idx = m_tabs.size() - 1;
        auto tab = m_tabs[tab_idx];
        if (!closeTabAction(tab_idx)) {
            return false;
        }
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    bool closeWindow = false;
    if (!hasModifiedFiles()) {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, tr("Exit ") + QApplication::applicationName(),
                                                                    tr("Are you sure?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes) {
            closeWindow = true;
            closeFile();
        }
    } else {
        if (closeFile()) {
            closeWindow = true;
        }
    }
    if (closeWindow) {
      updateSettings();
      e->accept();
      m_editor->deleteLater();
      m_editor = nullptr;
    } else {
      e->ignore();
    }
}

void MainWindow::updateSettings() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.beginGroup("splitter");
    settings.setValue("geometry", ui->splitter->saveGeometry());
    settings.setValue("state", ui->splitter->saveState());
    settings.endGroup();
    settings.endGroup();
}

bool MainWindow::hasModifiedFiles()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    QString allAutoSaveFileName;
    if (settings.contains("autosaveFile")) {
        allAutoSaveFileName = settings.value("autosaveFile").toString();
    }
    for (int idx = 0; idx < m_tabs.size(); ++idx) {
        auto tab = m_tabs[idx];
        if (!tab.undoStack()->isClean()) {
            return true;
        }
        if ((!tab.currentFile().fileName().isEmpty())&&(allAutoSaveFileName.contains(tab.currentFile().fileName()))) {
            return true;
        }
    }
    return false;
}

void MainWindow::on_actionSave_As_triggered()
{
    QString fname = m_currentFile.absoluteFilePath();
    QString path = m_defaultDirectory.absolutePath();
    if (!m_currentFile.fileName().isEmpty()) {
        path = m_currentFile.absoluteFilePath();
    }
    fname = QFileDialog::getSaveFileName(this, tr("Save File as ..."), path, tr("Panda files (*.panda)"));
    if (fname.isEmpty()) {
        return;
    }
    if (!fname.endsWith(".panda")) {
        fname.append(".panda");
    }
    setCurrentFile(QFileInfo(fname));
    save(m_currentFile.absoluteFilePath());
}

QFileInfo MainWindow::getCurrentFile() const
{
    return m_currentFile;
}

void MainWindow::setCurrentFile(const QFileInfo &file)
{
    setAutoSaveFileName(file);
    if (!file.fileName().isEmpty()) {
        ui->tabWidget_mainWindow->setTabText(m_current_tab, file.fileName());
        m_currentFile = file;
        m_tabs[m_current_tab].setCurrentFile(m_currentFile);
    }
    if (!m_loadedAutoSave) {
        COMMENT("Adding file to controller.", 0);
        emit addRecentFile(file.absoluteFilePath());
    }
    GlobalProperties::currentFile = m_currentFile.absoluteFilePath();
    COMMENT("Setting global current file.", 0);
    if (m_currentFile.exists()) {
        m_defaultDirectory = m_currentFile.dir();
    } else {
        m_defaultDirectory = QDir::home();
    }
}

void MainWindow::setAutoSaveFileName(const QFileInfo &file)
{
    COMMENT("Defining autosave path.", 0);
    if (file.exists()) {
        QDir autosavePath(QDir::temp());
        COMMENT("Autosave path set to the current file's directory, if there is one.", 0);
        autosavePath = file.dir();
        COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 0);
        m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath(file.baseName() + "XXXXXX.panda"));
        COMMENT("Setting current file to: " << file.absoluteFilePath().toStdString(), 0);
    } else {
        COMMENT("Default file does not exist: " << file.absoluteFilePath().toStdString(), 0);
        QDir autosavePath(QDir::temp());
        COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 0);
        m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath("XXXXXX.panda"));
        COMMENT("Setting current file to random file in tmp.", 0);
    }
}

void MainWindow::on_actionSelect_all_triggered()
{
    m_editor->selectAll();
}

void MainWindow::updateRecentICs() // Bug here... It is not showing and loading the recent boxes.
{
    ui->scrollAreaWidgetContents_Box->layout()->removeItem(ui->verticalSpacer_IC);
    for (ListItemWidget *item : qAsConst(icItemWidgets)) {
        item->deleteLater();
    }
    icItemWidgets.clear();
    //! Show recent files
    const QStringList files = m_ricController->getRecentFiles();
    for (const QString &file : files) {
        QPixmap pixmap(QString::fromUtf8(":/basic/box.png"));
        auto *item = new ListItemWidget(pixmap, ElementType::IC, file, this);
        icItemWidgets.append(item);
        ui->scrollAreaWidgetContents_Box->layout()->addWidget(item);
    }
    ui->scrollAreaWidgetContents_Box->layout()->addItem(ui->verticalSpacer_IC);
}

void MainWindow::closeTab(int tab) {
    if (m_tabs.size() == 1) {
        close();
    } else {
        closeTabAction(tab);
    }
}

bool MainWindow::closeTabAction(int tab)
{
    COMMENT("Closing tab " << QString::number(tab).toStdString() << ", #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    QString allAutoSaveFileName;
    QString autoSaveFileName;
    COMMENT("Reading autosave file names.", 0);
    if (settings.contains("autosaveFile")) {
        allAutoSaveFileName = settings.value("autosaveFile").toString();
        COMMENT("Verifying if this is a recovered autosave file.", 0);
        if (allAutoSaveFileName.contains(m_tabs[tab].currentFile().fileName())) {
            autoSaveFileName = m_tabs[tab].currentFile().absoluteFilePath();
        }
    }
    COMMENT("All auto save file names before closing tab: " << allAutoSaveFileName.toStdString(), 0);
    COMMENT("Checking if needs to save file.", 0);
    if ((!m_tabs[tab].undoStack()->isClean()) || (!autoSaveFileName.isEmpty())) {
        bool discardAutosaves = false;
        int ret = confirmSave(false);
        if (ret == QMessageBox::Yes) {
            if(!save(m_tabs[tab].currentFile().absoluteFilePath())) {
                if(closeTabAnyway()==QMessageBox::No) {
                    return false;
                } else {
                    discardAutosaves = true;
                }
            }
        } else if (ret == QMessageBox::Cancel) {
            return false;
        } else { // Close without saving. Deleting autosave if it was opened.
            discardAutosaves = true;
        }
        if (discardAutosaves) {
            if (!autoSaveFileName.isEmpty()) {
                COMMENT("Discarding recovered autosave file.", 0);
                allAutoSaveFileName.remove(autoSaveFileName+"\t");
                settings.setValue("autosaveFile", allAutoSaveFileName);
                COMMENT("All auto save file names after removing recovery file: " << allAutoSaveFileName.toStdString(), 0);
            }
            if (!m_tabs[tab].undoStack()->isClean()) {
                COMMENT("Discarding autosave modification.", 0);
                allAutoSaveFileName.remove(m_autoSaveFile[tab]->fileName()+"\t");
                settings.setValue("autosaveFile", allAutoSaveFileName);
                m_autoSaveFile[tab]->remove(); // This only removes the autosave file itself, not the QTempFile object.
                COMMENT("All auto save file names after removing autosave file: " << allAutoSaveFileName.toStdString(), 0);
            }
        }
    }
    COMMENT("Moving to other tab if closed tab is the current one, and if possible.", 0);
    selectTab((tab + 1) % m_tabs.size());
    COMMENT("Deleting tab and autosave", 0);
    removeAutosaveFile(tab); // This removes the QTempFile object allocation.
    m_tabs[tab].free();
    m_tabs.remove(tab);
    COMMENT("Removing undo and redo menus. Used only when closing the program with force_close = true.", 0);
    if (m_current_tab == tab) {
        ui->menuEdit->removeAction(m_undoAction[tab]);
        ui->menuEdit->removeAction(m_redoAction[tab]);
    }
    m_undoAction.remove(tab);
    m_redoAction.remove(tab);
    ui->tabWidget_mainWindow->removeTab(tab);
    if (m_current_tab >= m_tabs.size()) {
        m_current_tab = std::max(0, m_tabs.size() - 1);
    }
    COMMENT("Closed tab " << QString::number(tab).toStdString() << ", #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
    return true;
}

void MainWindow::selectTab(int tab) {
    COMMENT("Selecting tab: " << QString::number(tab).toStdString(), 0);
    if (tab != m_current_tab) {
        m_editor->getSimulationController()->stop();
        COMMENT("full screen.", 0);
        disconnect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
        m_fullscreenDlg = m_tabs[tab].fullScreenDlg();
        m_fullscreenView = m_tabs[tab].fullscreenView();
        COMMENT("editor.", 0);
        m_editor->selectWorkspace(&m_tabs[tab]);
        COMMENT("undo and redo menu.", 0);
        selectUndoRedoMenu(tab);
        COMMENT("files.", 0);
        m_currentFile = m_tabs[tab].currentFile();
        m_dolphinFileName = m_tabs[tab].dolphinFileName();
        COMMENT("selecting tab in UI...", 0);
        m_current_tab = tab;
        ui->widgetElementEditor->setScene(m_tabs[tab].scene());
        connect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
        COMMENT("emit circuithaschanged.", 0);
        m_editor->getSimulationController()->start();
        emit m_editor->circuitHasChanged();
    }
    COMMENT("New tab selected. BD filename: " << m_dolphinFileName.toStdString(), 0);
}

QString MainWindow::getOpenICFile()
{
    return QFileDialog::getOpenFileName(this, tr("Load File as IC"), m_defaultDirectory.absolutePath(), tr("Panda files (*.panda)"));
}

void MainWindow::on_actionOpen_IC_triggered()
{
    /* LOAD FILE AS IC */
    QString fname = getOpenICFile();
    if (fname.isEmpty()) {
        return;
    }
    QFile fl(fname);
    if (!fl.exists()) {
        std::cerr << tr("Error: This file does not exist: ").toStdString() << fname.toStdString() << std::endl;
        return;
    }
    if (fl.open(QFile::ReadOnly)) {
        emit addRecentIcFile(fname);
        fl.close();
        ui->statusBar->showMessage(tr("Loaded IC successfully."), 2000);
    } else {
        std::cerr << tr("Could not open file in ReadOnly mode : ").toStdString() << fname.toStdString() << "." << std::endl;
        ui->statusBar->showMessage(tr("Error loading IC file."), 2000);
    }
}

void MainWindow::on_lineEdit_textChanged(const QString &text)
{
    ui->searchLayout->removeItem(ui->VSpacer);
    for (ListItemWidget *item : qAsConst(searchItemWidgets)) {
        item->deleteLater();
    }
    searchItemWidgets.clear();
    m_firstResult = nullptr;
    if (text.isEmpty()) {
        ui->searchScrollArea->hide();
        ui->tabWidget->show();
    } else {
        ui->searchScrollArea->show();
        ui->tabWidget->hide();
        QList<Label *> ics = ui->tabWidget->findChildren<Label *>("label_ic");
        QRegularExpression regex(QString(".*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        QList<Label *> searchResults;
        QRegularExpression regex2(QString("^label_.*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        searchResults.append(ui->tabWidget->findChildren<Label *>(regex2));
        QList<Label *> allLabels = ui->tabWidget->findChildren<Label *>();
        for (Label *lb : qAsConst(allLabels)) {
            if (regex.match(lb->name()).hasMatch()) {
                if (!searchResults.contains(lb)) {
                    searchResults.append(lb);
                }
            }
        }
        for (auto *ic : qAsConst(ics)) {
            if (regex.match(ic->auxData()).hasMatch()) {
                searchResults.append(ic);
            }
        }
        for (auto *label : searchResults) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
            auto *item = new ListItemWidget(label->pixmap(Qt::ReturnByValue), label->elementType(), label->auxData());
#else
            auto *item = new ListItemWidget(*label->pixmap(), label->elementType(), label->auxData());
#endif
            if (!m_firstResult) {
                m_firstResult = item->getLabel();
            }
            ui->searchLayout->addWidget(item);
            searchItemWidgets.append(item);
        }
    }
    ui->searchLayout->addItem(ui->VSpacer);
}

void MainWindow::on_lineEdit_returnPressed()
{
    if (m_firstResult) {
        m_firstResult->startDrag();
        ui->lineEdit->clear();
    }
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    if(m_editor) {
        m_editor->getScene()->setSceneRect(m_editor->getScene()->sceneRect().united(m_fullscreenView->rect()));
    }
}

void MainWindow::on_actionReload_File_triggered()
{
    if (m_currentFile.exists()) {
        if (closeTabAction(m_current_tab)) {
            loadPandaFile(m_currentFile.absoluteFilePath());
        }
    }
}

void MainWindow::on_actionGates_triggered(bool checked)
{
    ui->actionWires->setEnabled(checked);
    if (!checked) {
        m_editor->showWires(checked);
    } else {
        m_editor->showWires(ui->actionWires->isChecked());
    }
    m_editor->showGates(checked);
}

bool MainWindow::exportToArduino(QString fname)
{
    try {
        if (fname.isEmpty()) {
            return false;
        }
        QVector<GraphicElement *> elements = m_editor->getScene()->getElements();
        SimulationController *sc = m_editor->getSimulationController();
        sc->stop();
        if (elements.isEmpty()) {
            return false;
        }
        if (!fname.endsWith(".ino")) {
            fname.append(".ino");
        }
        elements = ElementMapping::sortGraphicElements(elements);

        CodeGenerator arduino(QDir::home().absoluteFilePath(fname), elements);
        arduino.generate();
        sc->start();
        ui->statusBar->showMessage(tr("Arduino code successfully generated."), 2000);

        qDebug() << "Arduino code successfully generated.";
    } catch (std::runtime_error &e) {
        QMessageBox::warning(this, tr("Error"), tr("<strong>Error while exporting to Arduino code:</strong><br>%1").arg(e.what()));
        return false;
    }

    return true;
}

bool MainWindow::exportToWaveFormFile(const QString& fname)
{
    try {
        if (fname.isEmpty()) {
            return false;
        }
        m_bd = new BewavedDolphin(m_editor, this);
        if (!m_bd->createWaveform(m_dolphinFileName)) {
            std::cerr << ERRORMSG(tr("Could not open waveform file: %1.").arg(m_currentFile.fileName()).toStdString()) << std::endl;
            delete m_bd;
            return false;
        }
        m_bd->print();
    } catch (std::runtime_error &e) {
        QMessageBox::warning(this, tr("Error"), tr("<strong>Error while exporting to waveform file:</strong><br>%1").arg(e.what()));
        return false;
    }
    return true;
}

bool MainWindow::on_actionExport_to_Arduino_triggered()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Generate Arduino Code"), m_defaultDirectory.absolutePath(), tr("Arduino file (*.ino)"));
    return exportToArduino(fname);
}

void MainWindow::on_actionZoom_in_triggered()
{
    m_fullscreenView->gvzoom()->zoomIn();
}

void MainWindow::on_actionZoom_out_triggered()
{
    m_fullscreenView->gvzoom()->zoomOut();
}

void MainWindow::on_actionReset_Zoom_triggered()
{
    m_fullscreenView->gvzoom()->resetZoom();
}

void MainWindow::zoomChanged()
{
    ui->actionZoom_in->setEnabled(m_fullscreenView->gvzoom()->canZoomIn());
    ui->actionZoom_out->setEnabled(m_fullscreenView->gvzoom()->canZoomOut());
}

void MainWindow::updateRecentFileActions()
{
    QStringList files = m_rfController->getRecentFiles();
    int numRecentFiles = qMin(files.size(), static_cast<int>(RecentFilesController::MaxRecentFiles));
    if (numRecentFiles > 0) {
        ui->menuRecent_files->setEnabled(true);
    }
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActs[i]->setText(text);
        m_recentFileActs[i]->setData(files[i]);
        m_recentFileActs[i]->setVisible(true);
    }
    for (int i = numRecentFiles; i < RecentFilesController::MaxRecentFiles; ++i) {
        m_recentFileActs[i]->setVisible(false);
    }
}

void MainWindow::openRecentFile()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString fileName = action->data().toString();
        loadPandaFile(fileName);
    }
}

void MainWindow::createRecentFileActions()
{
    for (int i = 0; i < RecentFilesController::MaxRecentFiles; ++i) {
        m_recentFileActs[i] = new QAction(this);
        m_recentFileActs[i]->setVisible(false);
        connect(m_recentFileActs[i], &QAction::triggered, this, &MainWindow::openRecentFile);
        ui->menuRecent_files->addAction(m_recentFileActs[i]);
    }
    updateRecentFileActions();
}

void MainWindow::on_actionPrint_triggered()
{
    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), m_defaultDirectory.absolutePath(), tr("PDF files (*.pdf)"));
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
    QPainter p;
    if (!p.begin(&printer)) {
        QMessageBox::warning(this, tr("ERROR"), tr("Could not print this circuit to PDF."), QMessageBox::Ok);
        return;
    }
    m_editor->getScene()->render(&p, QRectF(), m_editor->getScene()->itemsBoundingRect().adjusted(-64, -64, 64, 64));
    p.end();
}

void MainWindow::on_actionExport_to_Image_triggered()
{
    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), m_defaultDirectory.absolutePath(), tr("PNG files (*.png)"));
    if (pngFile.isEmpty()) {
        return;
    }
    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }
    QRectF s = m_editor->getScene()->itemsBoundingRect().adjusted(-64, -64, 64, 64);
    QPixmap p(s.size().toSize());
    QPainter painter;
    painter.begin(&p);
    painter.setRenderHint(QPainter::Antialiasing);
    m_editor->getScene()->render(&painter, QRectF(), s);
    painter.end();
    QImage img = p.toImage();
    img.save(pngFile);
}

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
    ui->widgetElementEditor->retranslateUi();

    QList<ListItemWidget *> items = ui->tabWidget->findChildren<ListItemWidget *>();
    for (ListItemWidget *item : qAsConst(items)) {
        item->updateName();
    }
}

void MainWindow::loadTranslation(const QString& language)
{
    if (m_translator) {
        qApp->removeTranslator(m_translator);
    }
    m_translator = new QTranslator(this);
    if (m_translator->load(language)) {
        qApp->installTranslator(m_translator);
        retranslateUi();
    } else {
        QMessageBox::critical(this, "Error!", "Error loading translation!");
    }
}

void MainWindow::on_actionEnglish_triggered()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    QString language = "://wpanda_en.qm";
    settings.setValue("language", language);

    loadTranslation(language);
}

void MainWindow::on_actionPortuguese_triggered()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    QString language = "://wpanda_pt.qm";
    settings.setValue("language", language);
    loadTranslation(language);
}

void MainWindow::on_actionPlay_triggered(bool checked)
{
    if (checked) {
        m_editor->getSimulationController()->start();
    } else {
        m_editor->getSimulationController()->stop();
    }
    m_editor->getSimulationController()->updateAll();
}

void MainWindow::on_actionRename_triggered()
{
    m_editor->getElementEditor()->renameAction();
}

void MainWindow::on_actionChange_Trigger_triggered()
{
    m_editor->getElementEditor()->changeTriggerAction();
}

void MainWindow::on_actionClear_selection_triggered()
{
    if (m_fullscreenDlg->isVisible() && m_editor->getScene()->selectedItems().isEmpty()) {
        m_fullscreenDlg->accept();
    }
    m_editor->getScene()->clearSelection();
}

void MainWindow::populateMenu(QSpacerItem *spacer, const QString& names, QLayout *layout)
{
    QStringList list(names.split(","));
    layout->removeItem(spacer);
    for (QString name : qAsConst(list)) {
        name = name.trimmed().toUpper();
        ElementType type = ElementFactory::textToType(name);
        QPixmap pixmap(ElementFactory::getPixmap(type));
        auto *item = new ListItemWidget(pixmap, type, name, this);
        layout->addWidget(item);
    }
    layout->addItem(spacer);
    /*  layout->setSpacing(10); */
}

void MainWindow::populateLeftMenu()
{
    ui->tabWidget->setCurrentIndex(0);
    populateMenu(ui->verticalSpacer_InOut, "VCC,GND,BUTTON,SWITCH,ROTARY,CLOCK,LED,DISPLAY,DISPLAY14,BUZZER", ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(ui->verticalSpacer_Gates, "AND,OR,NOT,NAND,NOR,XOR,XNOR,MUX,DEMUX,NODE", ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(ui->verticalSpacer_Memory, "DFLIPFLOP,DLATCH,JKFLIPFLOP,SRFLIPFLOP,TFLIPFLOP", ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(ui->verticalSpacer_MISC, "TEXT,LINE", ui->scrollAreaWidgetContents_Misc->layout());
}

void MainWindow::on_actionFast_Mode_triggered(bool checked)
{
    setFastMode(checked);
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.setValue("fastMode", checked);
}

void MainWindow::on_actionWaveform_triggered()
{
    if (m_bd == nullptr) {
        m_bd = new BewavedDolphin(m_editor, this);
    }
    if (m_bd->createWaveform(m_dolphinFileName)) {
        COMMENT("BD filename: " << m_dolphinFileName.toStdString(), 0);
        m_bd->show();
    }
}

void MainWindow::on_actionPanda_Light_triggered()
{
    ThemeManager::globalMngr->setTheme(Theme::Panda_Light);
}

void MainWindow::on_actionPanda_Dark_triggered()
{
    ThemeManager::globalMngr->setTheme(Theme::Panda_Dark);
}

void MainWindow::updateTheme()
{
    switch (ThemeManager::globalMngr->theme()) {
    case Theme::Panda_Dark:
        ui->actionPanda_Dark->setChecked(true);
        break;
    case Theme::Panda_Light:
        ui->actionPanda_Light->setChecked(true);
        break;
    }
}

void MainWindow::on_actionFlip_horizontally_triggered()
{
    m_editor->flipH();
}

void MainWindow::on_actionFlip_vertically_triggered()
{
    m_editor->flipV();
}

void MainWindow::buildFullScreenDialog()
{
    m_fullscreenDlg = new QDialog(this);
    m_fullscreenView = new GraphicsView(this);
    m_fullscreenDlg->setWindowFlags(Qt::Window);
    auto *dlg_layout = new QHBoxLayout(m_fullscreenDlg);
    m_fullscreenDlg->addActions(actions());
    m_fullscreenDlg->addActions(ui->menuBar->actions());
    dlg_layout->setContentsMargins(0, 0, 0, 0);
    dlg_layout->setMargin(0);
    dlg_layout->addWidget(m_fullscreenView);
    m_fullscreenDlg->setLayout(dlg_layout);
    m_fullscreenDlg->setStyleSheet("QGraphicsView { border-style: none; }");
    m_fullscreenView->setScene(m_editor->getScene());
}

QString MainWindow::getDolphinFilename()
{
    return m_dolphinFileName;
}

void MainWindow::setDolphinFilename(const QString &filename)
{
    m_dolphinFileName = filename;
    m_tabs[m_current_tab].setDolphinFileName(filename);
}

void MainWindow::on_actionFullscreen_triggered() const
{
    if (m_fullscreenDlg->isVisible()) {
        m_fullscreenDlg->accept();
    } else {
        m_fullscreenDlg->showFullScreen();
        m_fullscreenDlg->exec();
    }
}

void MainWindow::autoSave()
{
    COMMENT("Starting autosave.", 2);
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    QString allAutoSaveFileName;
    if (settings.contains("autosaveFile")) {
        allAutoSaveFileName = settings.value("autosaveFile").toString();
        COMMENT("All auto save file names before autosaving: " << allAutoSaveFileName.toStdString(), 3);
        if (allAutoSaveFileName.contains(m_autoSaveFile[m_current_tab]->fileName())) {
          COMMENT("Removing current autosave file name.", 3);
          allAutoSaveFileName.remove(m_autoSaveFile[m_current_tab]->fileName());
          settings.setValue("autosaveFile", allAutoSaveFileName);
        }
    }
    COMMENT("All auto save file names after possibly removing autosave: " << allAutoSaveFileName.toStdString(), 3);
    if (m_editor->getUndoStack()->isClean()) {
        COMMENT("Undo stack is clean.", 3);
        if (m_autoSaveFile[m_current_tab]->exists()) {
            m_autoSaveFile[m_current_tab]->remove();
        }
    } else {
        COMMENT("Undo in not clear. Have to set autosave file.", 3);
        if (m_autoSaveFile[m_current_tab]->exists()) {
            COMMENT("Autosave file already exists. Delete it to update.", 3);
            m_autoSaveFile[m_current_tab]->remove();
            if (!m_currentFile.fileName().isEmpty()) {
                QDir autosavePath(QDir::temp());
                COMMENT("Autosave path set to the current file's directory, if there is one.", 3);
                autosavePath = m_currentFile.dir();
                COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 3);
                m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath(m_currentFile.baseName() + "XXXXXX.panda"));
                COMMENT("Setting current file to: " << m_currentFile.absoluteFilePath().toStdString(), 3);
            } else {
                COMMENT("Default value not set yet.", 3);
                QDir autosavePath(QDir::temp());
                COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 3);
                m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath("XXXXXX.panda"));
                COMMENT("Setting current file to random file in tmp.", 3);
            }
        }
        if (m_autoSaveFile[m_current_tab]->open()) {
            COMMENT("Writing to autosave file.", 3);
            QDataStream ds(m_autoSaveFile[m_current_tab]);
            QString autosaveFilename = m_autoSaveFile[m_current_tab]->fileName() + "\t";
            try {
                m_editor->save(ds, m_dolphinFileName);
                allAutoSaveFileName.append(autosaveFilename);
                settings.setValue("autosaveFile", allAutoSaveFileName);
                COMMENT("All auto save file names after adding autosave: " << allAutoSaveFileName.toStdString(), 3);
            } catch (std::runtime_error &e) {
                std::cerr << tr("Error autosaving project: ").toStdString() << e.what() << std::endl;
                m_autoSaveFile[m_current_tab]->close();
                m_autoSaveFile[m_current_tab]->remove();
            }
            m_autoSaveFile[m_current_tab]->close();
        }
    }
    COMMENT("Finished autosave.", 3);
}

void MainWindow::on_actionMute_triggered()
{
    m_editor->mute(ui->actionMute->isChecked());
    if (ui->actionMute->isChecked()) {
        ui->actionMute->setText(tr("Unmute"));
    } else {
        ui->actionMute->setText(tr("Mute"));
    }
}

void MainWindow::on_actionLabels_under_icons_triggered(bool checked)
{
    checked ? ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon) : ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void MainWindow::on_actionSave_Local_Project_triggered()
{
    QString fname = m_currentFile.absoluteFilePath();
    QString path = m_defaultDirectory.absolutePath();
    if (!m_currentFile.fileName().isEmpty()) {
        path = m_currentFile.absoluteFilePath();
    }
    fname = QFileDialog::getSaveFileName(this, tr("Save Local Project"), path, tr("Panda files (*.panda)"));
    if (fname.isEmpty()) {
        return;
    }
    if (!fname.endsWith(".panda")) {
        fname.append(".panda");
    }
    setCurrentFile(QFileInfo(fname));
    COMMENT("Saving ics and skins", 0);
    path = m_currentFile.absolutePath();
    if (!QDir(path + "/boxes").exists()) {
        bool dir_res = QDir().mkpath(path + "/boxes");
        if (!dir_res)
            std::cerr << tr("Error creating ICs directory.").toStdString() << std::endl;
    }
    if (!QDir(path + "/skins").exists()) {
        bool dir_res = QDir().mkpath(path + "/skins");
        if (!dir_res)
            std::cerr << tr("Error creating skins directory.").toStdString() << std::endl;
    }
    COMMENT("Saving ics and skins to local directories.", 0);
    if (!m_editor->saveLocal(path)) {
        std::cerr << tr("Error saving ICs.").toStdString() << std::endl;
        ui->statusBar->showMessage(tr("Could not save the local project."), 2000);
        return;
    }
    COMMENT("Saving main project.", 0);
    save(fname);
}
