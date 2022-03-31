// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

<<<<<<< HEAD:app/MainWindow.cpp
#include "MainWindow.h"

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
#include <QUndoView>

#include "arduino/codegenerator.h"
#include "BewavedDolphin.h"
=======
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "bewaveddolphin.h"
#include "codegenerator.h"
>>>>>>> master:app/mainwindow.cpp
#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicsview.h"
#include "graphicsviewzoom.h"
#include "icmanager.h"
#include "label.h"
#include "listitemwidget.h"
#include "settings.h"
#include "simulationcontroller.h"
#include "thememanager.h"
#include "workspace.h"

<<<<<<< HEAD:app/MainWindow.cpp
#include "ui_MainWindow.h"
=======
#include <QActionGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QSaveFile>
#include <QShortcut>
#include <QTranslator>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#define SKIPEMPTYPARTS QString::SkipEmptyParts
#else
#define SKIPEMPTYPARTS Qt::SkipEmptyParts
#endif
>>>>>>> master:app/mainwindow.cpp

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
    COMMENT("WiRedPanda Version = " << APP_VERSION << " OR " << GlobalProperties::version, 0);
    qDebug() << "WiRedPanda Version = " << APP_VERSION << " OR " << QString::number(GlobalProperties::version);
    ui->setupUi(this);
    ThemeManager::globalManager = new ThemeManager(this);

    COMMENT("Settings filename: " << Settings::fileName().toStdString(), 0);
    loadTranslation(Settings::value("language").toString());

    COMMENT("Building dialog within a new tab.", 0);
    m_current_tab = 0;
    createNewWorkspace();
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabBarClicked, this, &MainWindow::selectTab);
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    COMMENT("Restoring geometry and setting zoom controls.", 0);
    Settings::beginGroup("MainWindow");
    restoreGeometry(Settings::value("geometry").toByteArray());
    restoreState(Settings::value("windowState").toByteArray());
    Settings::beginGroup("splitter");
    ui->splitter->restoreGeometry(Settings::value("geometry").toByteArray());
    ui->splitter->restoreState(Settings::value("state").toByteArray());
    ui->actionExport_to_Arduino->setEnabled(false);
    Settings::endGroup();
    Settings::endGroup();
    QList<QKeySequence> zoom_in_shortcuts;
    zoom_in_shortcuts << QKeySequence("Ctrl++") << QKeySequence("Ctrl+=");
    ui->actionZoom_in->setShortcuts(zoom_in_shortcuts);

    COMMENT("Preparing theme and UI modes.", 0);
    auto *themeGroup = new QActionGroup(this);
    const auto actions = ui->menuTheme->actions();
    for (auto *action : actions) {
        themeGroup->addAction(action);
    }
    themeGroup->setExclusive(true);
    connect(ThemeManager::globalManager, &ThemeManager::themeChanged, this, &MainWindow::updateTheme);
    connect(ThemeManager::globalManager, &ThemeManager::themeChanged, m_editor, &Editor::updateTheme);
    ThemeManager::globalManager->initialize();
    setFastMode(Settings::value("fastMode").toBool());

    COMMENT("Setting left side menus.", 0);
    m_editor->setElementEditor(ui->widgetElementEditor);
    ui->searchScrollArea->hide();
    auto *shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(shortcut, &QShortcut::activated, ui->lineEdit, qOverload<>(&QWidget::setFocus));
    populateLeftMenu();
<<<<<<< HEAD:app/MainWindow.cpp
    
    
    /* Connections for slot Menu File this class*/
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::actionExitTriggered);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::actionNewTriggered);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::actionOpenTriggered);
    connect(ui->actionOpen_IC, &QAction::triggered, this, &MainWindow::actionOpenIcTriggered);
    connect(ui->actionReload_File, &QAction::triggered, this, &MainWindow::actionReloadFileTriggered);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::actionSaveTriggered);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::actionSaveAsTriggered);
    connect(ui->actionSave_Local_Project, &QAction::triggered, this, &MainWindow::actionSaveLocalProjectTriggered);
    connect(ui->actionExport_to_Arduino, &QAction::triggered, this, &MainWindow::actionExportToArduinoTriggered);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::actionPrintTriggered);
    connect(ui->actionExport_to_Image, &QAction::triggered, this, &MainWindow::actionExportToImageTriggered);
    
    
    /* Connections for slot Menu Edit this class */
    connect(ui->actionRename, &QAction::triggered, this, &MainWindow::actionRenameTriggered);
    connect(ui->actionChange_Trigger, &QAction::triggered, this, &MainWindow::actionChangeTriggerTriggered);
    connect(ui->actionRotate_left, &QAction::triggered, this, &MainWindow::actionRotateLeftTriggered);
    connect(ui->actionRotate_right, &QAction::triggered, this, &MainWindow::actionRotateRightTriggered);
    connect(ui->actionFlip_horizontally, &QAction::triggered, this, &MainWindow::actionFlipHorizontallyTriggered);
    connect(ui->actionFlip_vertically, &QAction::triggered, this, &MainWindow::actionFlipVerticallyTriggered);
    connect(ui->actionSelect_all, &QAction::triggered, this, &MainWindow::actionSelectAllTriggered);
    connect(ui->actionClear_selection, &QAction::triggered, this, &MainWindow::actionClearSelectionTriggered);
    
    
    
    /* Connections for slot Menu View this class*/
    connect(ui->actionPanda_Light, &QAction::triggered, this, &MainWindow::actionPandaLightTriggered);
    connect(ui->actionPanda_Dark, &QAction::triggered, this, &MainWindow::actionPandaDarkTriggered);
    connect(ui->actionZoom_in, &QAction::triggered, this, &MainWindow::actionZoomInTriggered);
    connect(ui->actionZoom_out, &QAction::triggered, this, &MainWindow::actionZoomOutTriggered);
    connect(ui->actionReset_Zoom, &QAction::triggered, this, &MainWindow::actionResetZoomTriggered);
    connect(ui->actionWires, &QAction::triggered, this, &MainWindow::actionWiresTriggered);
    connect(ui->actionGates, &QAction::triggered, this, &MainWindow::actionGatesTriggered);
    connect(ui->actionFullscreen, &QAction::triggered, this, &MainWindow::actionFullScreenTriggered);
    connect(ui->actionLabels_under_icons, &QAction::triggered, this, &MainWindow::actionLabelsUnderIconsTriggered);
    
    
    /* Connections for slot Menu Language this class*/
    connect(ui->actionEnglish, &QAction::triggered, this, &MainWindow::actionEnglishTriggered);
    connect(ui->actionPortuguese, &QAction::triggered, this, &MainWindow::actionPortugueseTriggered);
    
    /* Connections for slot Menu About this class*/
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::actionAboutTriggered);
    connect(ui->actionAbout_Qt, &QAction::triggered, this, &MainWindow::actionAboutQtTriggered);
    
     /* Connections for slot Menu Simulation this class*/
    connect(ui->actionPlay, &QAction::triggered, this, &MainWindow::actionPlayTriggered);
    connect(ui->actionWaveform, &QAction::triggered, this, &MainWindow::actionWaveFormTriggered);
    connect(ui->actionMute, &QAction::triggered, this, &MainWindow::actionMuteTriggered);
    
    
    /* Connections for slot LineEdit this class*/
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::lineEditReturnPressed);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainWindow::lineEditTextChanged);
    
    
    
    
    
=======

    COMMENT("Setting directories and autosave file.", 0);
    setCurrentFile({});

    COMMENT("Creating edit menu funcionalities.", 0);
    connect(ui->actionCopy, &QAction::triggered, m_editor, &Editor::copyAction);
    connect(ui->actionCut, &QAction::triggered, m_editor, &Editor::cutAction);
    connect(ui->actionPaste, &QAction::triggered, m_editor, &Editor::pasteAction);
    connect(ui->actionDelete, &QAction::triggered, m_editor, &Editor::deleteAction);
    connect(ui->actionAbout_this_version, &QAction::triggered, this, &MainWindow::aboutThisVersion);
    ui->actionWaveform->setDisabled(true);

    COMMENT("Setting up zoom and screen funcionalities.", 0);
    connect(m_editor, &Editor::scroll, this, &MainWindow::scrollView);
    connect(m_editor, &Editor::circuitHasChanged, this, &MainWindow::autoSave);
    connect(m_editor, &Editor::circuitAppearenceHasChanged, this, &MainWindow::autoSave);
    m_fullscreenView->setCacheMode(QGraphicsView::CacheBackground);

    COMMENT("Loading recent file list.", 0);
    m_rfController = new RecentFilesController("recentFileList", this, true);
    connect(this, &MainWindow::addRecentFile, m_rfController, &RecentFilesController::addRecentFile);
    m_firstResult = nullptr;
    // updateRecentICs();
    createRecentFileActions();
    connect(m_rfController, &RecentFilesController::recentFilesUpdated, this, &MainWindow::updateRecentFileActions);

    COMMENT("Checking for autosave file recovery.", 0);
    loadAutoSaveFiles(filename);
    COMMENT("Checking playing simulation.", 0);
    ui->actionPlay->setChecked(true);

    COMMENT("Connecting change tab shortcut to tab selection function.", 0);
    auto *nextTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), ui->tabWidget_mainWindow);
    connect(nextTabShortcut, &QShortcut::activated, this, &MainWindow::selectNextTab);
    auto *prevTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backtab), ui->tabWidget_mainWindow);
    connect(prevTabShortcut, &QShortcut::activated, this, &MainWindow::selectPreviousTab);

    COMMENT("Window title.", 0);
    setWindowTitle("WiRedPanda " + QString(APP_VERSION));
}

void MainWindow::selectNextTab()
{
    COMMENT("Forward: " << QString::number(m_current_tab).toStdString(), 0);
    selectTab((m_current_tab + 1) % m_tabs.size());
    ui->tabWidget_mainWindow->setCurrentIndex(m_current_tab);
    COMMENT("Forward end: " << QString::number(m_current_tab).toStdString(), 0);
}

void MainWindow::selectPreviousTab()
{
    COMMENT("Backwards: " << QString::number(m_current_tab).toStdString(), 0);
    selectTab((m_current_tab + m_tabs.size() - 1) % m_tabs.size());
    ui->tabWidget_mainWindow->setCurrentIndex(m_current_tab);
    COMMENT("Backwards end: " << QString::number(m_current_tab).toStdString(), 0);
}

void MainWindow::loadAutoSaveFiles(const QString &filename)
{
    if (Settings::contains("autosaveFile")) {
        COMMENT("autosave files exist in config file.", 0);
        bool yes_to_all = false;
        bool no_to_all = false;
        QString allAutoSaveFileNames(Settings::value("autosaveFile").toString());
        QStringList autoSaveFileNameList(allAutoSaveFileNames.split("\t", SKIPEMPTYPARTS));
        COMMENT("all files: " << allAutoSaveFileNames.toStdString(), 0);
        for (const auto &autoSaveFileName : qAsConst(autoSaveFileNameList)) {
            if ((QFile(autoSaveFileName).exists()) && (autoSaveFileName != filename)) {
                COMMENT("Autosave exists and was not open. Trying to recover.", 0);
                int ret = QMessageBox::No;
                if ((!yes_to_all) && (!no_to_all)) {
                    ret = recoverAutoSaveFile(autoSaveFileName);
                    if (ret == QMessageBox::YesToAll) {
                        yes_to_all = true;
                    }
                    if (ret == QMessageBox::NoToAll) {
                        no_to_all = true;
                    }
                }
                if ((yes_to_all) || (ret == QMessageBox::Yes)) {
                    if (!loadPandaFile(autoSaveFileName)) {
                        ret = autoSaveFileDeleteAnyway(autoSaveFileName);
                        if (ret == QMessageBox::Yes) {
                            allAutoSaveFileNames.remove(autoSaveFileName + "\t");
                            Settings::setValue("autosaveFile", allAutoSaveFileNames);
                        }
                    }
                }
            } else if ((QFile(autoSaveFileName).exists()) && (autoSaveFileName == filename)) {
                COMMENT("Loading autosave!", 0);
                m_loadedAutoSave = true;
            } else if (!QFile(autoSaveFileName).exists()) {
                COMMENT("Removing autosave file name from config that does not exist.", 0);
                allAutoSaveFileNames.remove(autoSaveFileName + "\t");
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
            }
        }
    } else {
        COMMENT("autosave files do no exist in config file.", 0);
    }
}

void MainWindow::createNewTab()
{
    COMMENT("Dialog built. Now adding tab. #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
    auto *new_tab = new QWidget(ui->tabWidget_mainWindow);
    new_tab->setLayout(m_fullscreenDlg->layout());
    new_tab->layout()->addWidget(m_fullscreenView);
    ui->tabWidget_mainWindow->addTab(new_tab, "");
    COMMENT("Storing tab pointers.", 0);
    m_tabs.push_back(WorkSpace(m_fullscreenDlg, m_fullscreenView, m_editor));
    COMMENT("Finished #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
}

void MainWindow::createAutosaveFile()
{
    auto **new_vector = new QTemporaryFile *[m_tabs.size()];
    for (int idx = 0; idx < m_tabs.size() - 1; ++idx) {
        new_vector[idx] = m_autoSaveFile[idx];
    }
    new_vector[m_tabs.size() - 1] = new QTemporaryFile();
    delete[] m_autoSaveFile;
    m_autoSaveFile = new_vector;
}

void MainWindow::removeAutosaveFile(int tab)
{
    QTemporaryFile **new_vector = nullptr;
    if (m_tabs.size() > 1) {
        new_vector = new QTemporaryFile *[m_tabs.size() - 1];
        for (int idx = 0; idx < tab; ++idx) {
            new_vector[idx] = m_autoSaveFile[idx];
        }
        for (int idx = tab + 1; idx < m_tabs.size(); ++idx) {
            new_vector[idx - 1] = m_autoSaveFile[idx];
        }
    }
    delete m_autoSaveFile[tab];
    delete[] m_autoSaveFile;
    m_autoSaveFile = new_vector;
}

void MainWindow::createUndoRedoMenus()
{
    m_undoAction.push_back(m_editor->getUndoStack()->createUndoAction(this, tr("&Undo")));
    m_undoAction[m_undoAction.size() - 1]->setIcon(QIcon(QPixmap(":/toolbar/undo.png")));
    m_undoAction[m_undoAction.size() - 1]->setShortcuts(QKeySequence::Undo);
    m_redoAction.push_back(m_editor->getUndoStack()->createRedoAction(this, tr("&Redo")));
    m_redoAction[m_undoAction.size() - 1]->setIcon(QIcon(QPixmap(":/toolbar/redo.png")));
    m_redoAction[m_undoAction.size() - 1]->setShortcuts(QKeySequence::Redo);
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), m_undoAction[m_undoAction.size() - 1]);
    ui->menuEdit->insertAction(m_undoAction[m_undoAction.size() - 1], m_redoAction[m_undoAction.size() - 1]);
    connect(m_editor->getUndoStack(), &QUndoStack::indexChanged, m_editor, &Editor::checkUpdateRequest);
}

void MainWindow::removeUndoRedoMenu()
{
    ui->menuEdit->removeAction(m_undoAction[m_current_tab]);
    ui->menuEdit->removeAction(m_redoAction[m_current_tab]);
    disconnect(m_editor->getUndoStack(), &QUndoStack::indexChanged, m_editor, &Editor::checkUpdateRequest);
}

void MainWindow::addUndoRedoMenu(int tab)
{
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), m_undoAction[tab]);
    ui->menuEdit->insertAction(m_undoAction[tab], m_redoAction[tab]);
    connect(m_editor->getUndoStack(), &QUndoStack::indexChanged, m_editor, &Editor::checkUpdateRequest);
>>>>>>> master:app/mainwindow.cpp
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

void MainWindow::actionExitTriggered()
{
    close();
}

bool MainWindow::save(QString fname)
{
    COMMENT("fname: " << fname.toStdString(), 0);
    COMMENT("Getting autosave settings info.", 0);
    QString allAutoSaveFileNames;
    if (Settings::contains("autosaveFile")) {
        allAutoSaveFileNames = Settings::value("autosaveFile").toString();
    }
    COMMENT("All auto save file names before save: " << allAutoSaveFileNames.toStdString(), 0);
    COMMENT("Checking if it is an autosave file or new a new project, and ask for a filename.", 0);
    QString autoSaveFileName;
    if ((fname.isEmpty()) || ((!allAutoSaveFileNames.isEmpty()) && (allAutoSaveFileNames.contains(fname)))) {
        COMMENT("Should open window!", 0);
        autoSaveFileName = fname;
        if (m_currentFile.fileName().isEmpty()) {
            fname = QFileDialog::getSaveFileName(this, tr("Save File"), m_defaultDirectory, tr("Panda files (*.panda)"));
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
        setCurrentFile(fname);
        ui->statusBar->showMessage(tr("Saved file successfully."), 4000);
        m_editor->getUndoStack()->setClean();
        COMMENT("Remove from autosave list recovered file that has been saved.", 0)
        if (!autoSaveFileName.isEmpty()) {
            allAutoSaveFileNames.remove(autoSaveFileName + "\t");
            Settings::setValue("autosaveFile", allAutoSaveFileNames);
            COMMENT("All auto save file names after removing recovered: " << allAutoSaveFileNames.toStdString(), 0);
        }
        COMMENT("Remove autosave from settings and deleting it.", 0);
        if (m_autoSaveFile[m_current_tab]->isOpen()) {
            allAutoSaveFileNames.remove(m_autoSaveFile[m_current_tab]->fileName() + "\t");
            Settings::setValue("autosaveFile", allAutoSaveFileNames + "\n");
            m_autoSaveFile[m_current_tab]->remove();
            COMMENT("All auto save file names after removing autosave: " << allAutoSaveFileNames.toStdString(), 0);
        }
        return true;
    }
    std::cerr << QString(tr("Could not save file: ") + fl.errorString() + ".").toStdString() << std::endl;
    return false;
}

void MainWindow::show()
{
    QMainWindow::show();
    if (!Settings::contains("WhatsNew4")) {
        aboutThisVersion();
    }
}

void MainWindow::aboutThisVersion()
{
    COMMENT("What is new message box.", 0);
    auto *cb = new QCheckBox(tr("Don't show this again."));
    cb->setChecked(Settings::contains("WhatsNew4"));
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
    msgBox.setCheckBox(cb);
    QObject::connect(cb, &QCheckBox::stateChanged, this, [](int state) {
        if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
            Settings::setValue("WhatsNew4", "true");
        } else {
            Settings::remove("WhatsNew4");
        }
    });
    msgBox.exec();
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


int MainWindow::recoverAutoSaveFile(const QString &autosaveFilename)
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);
    msgBox.setText(QString(tr("An autosave file was found. Do you want to load it?\n") + autosaveFilename));
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
    QString fname = m_currentFile.absoluteFilePath();
    if (fname.isEmpty()) {
        fname = tr("New Project");
    }
    msgBox.setText(fname + tr(" has been modified. Do you want to save your changes?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}

int MainWindow::autoSaveFileDeleteAnyway(const QString &autosaveFilename)
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setText(QString(tr("Error while opening autosave file. Do you want to delete it ?\n Autosave: ") + autosaveFilename));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}

<<<<<<< HEAD:app/MainWindow.cpp
=======
void MainWindow::on_actionNew_triggered()
{
    createNewWorkspace();
    setCurrentFile({});
}
>>>>>>> master:app/mainwindow.cpp

void MainWindow::actionWiresTriggered(bool checked)
{
    m_editor->showWires(checked);
}

void MainWindow::actionRotateRightTriggered()
{
    m_editor->rotate(true);
}

void MainWindow::actionRotateLeftTriggered()
{
    m_editor->rotate(false);
}

bool MainWindow::loadPandaFile(const QString &fname)
{
    QFile fl(fname);
    if (!fl.exists()) {
        QMessageBox::warning(this, tr("Error!"), tr("File \"%1\" does not exist!").arg(fname), QMessageBox::Ok, QMessageBox::NoButton);
        std::cerr << tr("Error: This file does not exist: ").toStdString() << fname.toStdString() << std::endl;
        return false;
    }
    COMMENT("File exists.", 0);
    if (fl.open(QFile::ReadOnly)) {
        COMMENT("File opened: " << fname.toStdString(), 0);
        const int current_tab = m_current_tab; // Stored to possibly recover it in case of error.
        createNewWorkspace();
        try {
            COMMENT("Current file set.", 0);
            setCurrentFile(fname);
            COMMENT("Loading in editor.", 0);
            QDataStream ds(&fl);
            m_editor->load(ds);
            updateICList();
        } catch (std::runtime_error &e) {
            std::cerr << tr("Error loading project: ").toStdString() << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error!"), tr("Could not open file.\nError: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::NoButton);
            closeTab(m_tabs.size() - 1);
            selectTab(current_tab);
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

void MainWindow::scrollView(int dx, int dy) const
{
    m_fullscreenView->scroll(dx, dy);
}



void MainWindow::actionNewTriggered()
{
    if (closeFile()) {
        clear();
    }
}

void MainWindow::actionOpenTriggered()
{
    if (m_defaultDirectory.isEmpty()) {
        m_defaultDirectory = "./examples/";
    }

    QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), m_defaultDirectory, tr("Panda files (*.panda)"));
    if (fname.isEmpty()) {
        return;
    }
    loadPandaFile(fname);
}

void MainWindow::actionOpenIcTriggered()
{
    /* LOAD FILE AS IC */
    QString fname = getOpenICFile();
    if (fname.isEmpty()) {
        return;
    }
    QFile fl(fname);
    if (!fl.exists()) {
        std::cerr << tr("Error: This file does not exists: ").toStdString() << fname.toStdString() << std::endl;
        return;
    }
    if (fl.open(QFile::ReadOnly)) {
        emit addRecentIcFile(fname);
    } else {
        std::cerr << tr("Could not open file in ReadOnly mode : ").toStdString() << fname.toStdString() << "." << std::endl;
        return;
    }
    fl.close();

    ui->statusBar->showMessage(tr("Loaded ic successfully."), 2000);
}




void MainWindow::actionReloadFileTriggered()
{
    if (currentFile.exists()) {
        if (closeFile()) {
            loadPandaFile(currentFile.absoluteFilePath());
        }
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
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], &QAction::triggered, this, &MainWindow::openRecentFile);
        ui->menuRecent_files->addAction(recentFileActs[i]);
    }
    updateRecentFileActions();
    for (int i = 0; i < RecentFilesController::MaxRecentFiles; ++i) {
        ui->menuRecent_files->addAction(recentFileActs[i]);
    }
}


void MainWindow::actionSaveTriggered()
{
    save(m_currentFile.absoluteFilePath());
}


void MainWindow::actionSaveAsTriggered()
{
    QString fname = currentFile.absoluteFilePath();
    QString path = defaultDirectory.absolutePath();
    if (!currentFile.fileName().isEmpty()) {
        path = currentFile.absoluteFilePath();
    }
    fname = QFileDialog::getSaveFileName(this, tr("Save File as ..."), path, tr("Panda files (*.panda)"));
    if (fname.isEmpty()) {
        return;
    }
    if (!fname.endsWith(".panda")) {
        fname.append(".panda");
    }
    setCurrentFile(QFileInfo(fname));
    save();
}


void MainWindow::actionSaveLocalProjectTriggered()
{
    QString fname = currentFile.absoluteFilePath();
    QString path = defaultDirectory.absolutePath();
    if (!currentFile.fileName().isEmpty()) {
        path = currentFile.absoluteFilePath();
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
    // Saves ics ans skins locally here.
    path = currentFile.absolutePath();
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
    if (!editor->saveLocal(path)) {
        std::cerr << tr("Error saving ICs.").toStdString() << std::endl;
        ui->statusBar->showMessage(tr("Could not save the local project."), 2000);
        return;
    }
    COMMENT("Saving main project.", 0);
    save();
}


bool MainWindow::actionExportToArduinoTriggered()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Generate Arduino Code"), defaultDirectory.absolutePath(), tr("Arduino file (*.ino)"));
    return exportToArduino(fname);
}



void MainWindow::actionPrintTriggered()
{
    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), defaultDirectory.absolutePath(), tr("PDF files (*.pdf)"));
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
    editor->getScene()->render(&p, QRectF(), editor->getScene()->itemsBoundingRect().adjusted(-64, -64, 64, 64));
    p.end();
}



void MainWindow::actionExportToImageTriggered()
{
    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), defaultDirectory.absolutePath(), tr("PNG files (*.png)"));
    if (pngFile.isEmpty()) {
        return;
    }
    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }
    QRectF s = editor->getScene()->itemsBoundingRect().adjusted(-64, -64, 64, 64);
    QPixmap p(s.size().toSize());
    QPainter painter;
    painter.begin(&p);
    painter.setRenderHint(QPainter::Antialiasing);
    editor->getScene()->render(&painter, QRectF(), s);
    painter.end();
    QImage img = p.toImage();
    img.save(pngFile);
}


void MainWindow::actionAboutTriggered()
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

void MainWindow::actionAboutQtTriggered()
{
    QMessageBox::aboutQt(this);
}

bool MainWindow::closeFile()
{
    while (!m_tabs.empty()) {
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
        auto reply =
            QMessageBox::question(
                this,
                tr("Exit ") + QApplication::applicationName(),
                tr("Are you sure?"),
                QMessageBox::Cancel | QMessageBox::Yes,
                QMessageBox::Yes);

        if (reply == QMessageBox::Yes) {
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

<<<<<<< HEAD:app/MainWindow.cpp
=======
void MainWindow::updateSettings()
{
    Settings::beginGroup("MainWindow");
    Settings::setValue("geometry", saveGeometry());
    Settings::setValue("windowState", saveState());
    Settings::beginGroup("splitter");
    Settings::setValue("geometry", ui->splitter->saveGeometry());
    Settings::setValue("state", ui->splitter->saveState());
    Settings::endGroup();
    Settings::endGroup();
}

bool MainWindow::hasModifiedFiles()
{
    QString allAutoSaveFileNames;
    if (Settings::contains("autosaveFile")) {
        allAutoSaveFileNames = Settings::value("autosaveFile").toString();
    }
    for (auto tab : m_tabs) {
        if (!tab.undoStack()->isClean()) {
            return true;
        }
        if ((!tab.currentFile().fileName().isEmpty()) && (allAutoSaveFileNames.contains(tab.currentFile().fileName()))) {
            return true;
        }
    }
    return false;
}

void MainWindow::on_actionSave_As_triggered()
{
    QString path = m_defaultDirectory;
    if (!m_currentFile.fileName().isEmpty()) {
        path = m_currentFile.absoluteFilePath();
    }
    QString fname = QFileDialog::getSaveFileName(this, tr("Save File as ..."), path, tr("Panda files (*.panda)"));
    if (fname.isEmpty()) {
        return;
    }
    if (!fname.endsWith(".panda")) {
        fname.append(".panda");
    }
    save(fname);
}
>>>>>>> master:app/mainwindow.cpp

QFileInfo MainWindow::getCurrentFile() const
{
    return m_currentFile;
}

QDir MainWindow::getCurrentDir() const
{
    return m_currentFile.absoluteDir();
}

void MainWindow::setCurrentFile(const QString &fname)
{
    QFileInfo file(fname);
    setAutoSaveFileName(file);
    if (!file.fileName().isEmpty()) {
        ui->tabWidget_mainWindow->setTabText(m_current_tab, file.fileName());
    } else {
        ui->tabWidget_mainWindow->setTabText(m_current_tab, tr("New Project"));
    }
    m_currentFile = file;
    if (file.exists()) {
        updateICList();
    }
    m_tabs[m_current_tab].setCurrentFile(m_currentFile);
    if (!m_loadedAutoSave) {
        COMMENT("Adding file to controller.", 0);
        emit addRecentFile(file.absoluteFilePath());
    }
    COMMENT("Setting global current file and dir", 0);
    GlobalProperties::currentFile = m_currentFile.absoluteFilePath();
    setCurrentDir();
}

void MainWindow::setAutoSaveFileName(const QFileInfo &file)
{
    COMMENT("Defining autosave path.", 0);
    if (file.exists()) {
        QDir autosavePath(QDir::temp());
        COMMENT("Autosave path set to the current file's directory, if there is one.", 0);
        autosavePath = file.dir();
        COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 0);
        m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath("." + file.baseName() + "XXXXXX.panda"));
        COMMENT("Setting current file to: " << file.absoluteFilePath().toStdString(), 0);
    } else {
        COMMENT("Default file does not exist: " << file.absoluteFilePath().toStdString(), 0);
        QDir autosavePath(QDir::temp());
        COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 0);
        m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
        COMMENT("Setting current file to random file in tmp.", 0);
    }
}

void MainWindow::actionSelectAllTriggered()
{
    m_editor->selectAll();
}

void MainWindow::updateICList()
{
    ui->scrollAreaWidgetContents_Box->layout()->removeItem(ui->verticalSpacer_IC);
    for (auto *item : qAsConst(icItemWidgets)) {
        item->deleteLater();
    }
    icItemWidgets.clear();
    if (m_currentFile.exists()) {
        COMMENT("Show files.", 0);
        QDir directory(m_currentFile.absoluteDir());
        QStringList files = directory.entryList({"*.panda", "*.PANDA"}, QDir::Files);
        files.removeAll(m_currentFile.fileName());
        for (int i = files.size() - 1; i >= 0; --i) {
            if (files[i][0] == '.') {
                files.removeAt(i);
            }
        }
        COMMENT("Files: " << files.join(", ").toStdString(), 0);
        for (const QString &file : qAsConst(files)) {
            QPixmap pixmap(QString::fromUtf8(":/basic/box.png"));
            auto *item = new ListItemWidget(pixmap, ElementType::IC, file, this);
            icItemWidgets.append(item);
            ui->scrollAreaWidgetContents_Box->layout()->addWidget(item);
        }
    }
    ui->scrollAreaWidgetContents_Box->layout()->addItem(ui->verticalSpacer_IC);
}

void MainWindow::closeTab(int tab)
{
<<<<<<< HEAD:app/MainWindow.cpp
    return QFileDialog::getOpenFileName(this, tr("Load File as IC"), defaultDirectory.absolutePath(), tr("Panda files (*.panda)"));
}


<<<<<<< HEAD:app/MainWindow.cpp
    ui->statusBar->showMessage(tr("Loaded ic successfully."), 2000);
=======
    ui->statusBar->showMessage(tr("Loaded IC successfully."), 2000);
=======
    if (m_tabs.size() == 1) {
        close();
    } else {
        closeTabAction(tab);
    }
}

bool MainWindow::closeTabAction(int tab)
{
    COMMENT("Closing tab " << QString::number(tab).toStdString() << ", #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
    QString allAutoSaveFileNames;
    QString autoSaveFileName;
    COMMENT("Reading autosave file names.", 0);
    if (Settings::contains("autosaveFile")) {
        allAutoSaveFileNames = Settings::value("autosaveFile").toString();
        COMMENT("Verifying if this is a recovered autosave file.", 0);
        if (allAutoSaveFileNames.contains(m_tabs[tab].currentFile().fileName())) {
            autoSaveFileName = m_tabs[tab].currentFile().absoluteFilePath();
        }
    }
    COMMENT("All auto save file names before closing tab: " << allAutoSaveFileNames.toStdString(), 0);
    COMMENT("Checking if needs to save file.", 0);
    if ((!m_tabs[tab].undoStack()->isClean()) || (!autoSaveFileName.isEmpty())) {
        selectTab(tab);
        bool discardAutosaves = false;
        int ret = confirmSave(false);
        if (ret == QMessageBox::Yes) {
            if (!save(m_tabs[tab].currentFile().absoluteFilePath())) {
                if (closeTabAnyway() == QMessageBox::No) {
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
                allAutoSaveFileNames.remove(autoSaveFileName + "\t");
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
                COMMENT("All auto save file names after removing recovery file: " << allAutoSaveFileNames.toStdString(), 0);
            }
            if (!m_tabs[tab].undoStack()->isClean()) {
                COMMENT("Discarding autosave modification.", 0);
                allAutoSaveFileNames.remove(m_autoSaveFile[tab]->fileName() + "\t");
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
                m_autoSaveFile[tab]->remove(); // This only removes the autosave file itself, not the QTempFile object.
                COMMENT("All auto save file names after removing autosave file: " << allAutoSaveFileNames.toStdString(), 0);
            }
        }
    }
    COMMENT("Moving to other tab if closed tab is the current one and not the only one. Othewise, if has just one tab, disconnects it from the UI.", 0);
    if ((tab == m_current_tab) && (m_tabs.size() != 1)) {
        selectTab((tab + 1) % m_tabs.size());
    } else if (m_tabs.size() == 1) {
        disconnectTab();
    }
    COMMENT("Deleting tab and autosave", 0);
    removeAutosaveFile(tab); // This removes the QTempFile object allocation.
    m_tabs[tab].free();
    m_tabs.remove(tab);
    m_undoAction.remove(tab);
    m_redoAction.remove(tab);
    ui->tabWidget_mainWindow->removeTab(tab);
    COMMENT("Adjusting m_current_tab if removed tab lays before the current one.", 0);
    if (m_current_tab > tab) {
        m_current_tab--;
    }
    COMMENT("Closed tab " << QString::number(tab).toStdString() << ", #tabs: " << m_tabs.size() << ", current tab: " << m_current_tab, 0);
    return true;
}

void MainWindow::disconnectTab()
{
    COMMENT("Stopping simulation controller and event handling.", 0);
    m_editor->setHandlingEvents(false);
    m_editor->getSimulationController()->stop();
    COMMENT("Disconnecting zoom from UI controllers.", 0);
    disconnect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
    COMMENT("Removing undo and redo actions from UI menu.", 0);
    removeUndoRedoMenu();
    COMMENT("Disabling beWavedDolphin action.", 0);
}

void MainWindow::setCurrentDir()
{
    if (m_currentFile.exists()) {
        m_defaultDirectory = m_currentFile.dir().absolutePath();
        ui->actionWaveform->setEnabled(true);
    } else {
        ui->actionWaveform->setDisabled(true);
    }
}

void MainWindow::connectTab(int tab)
{
    COMMENT("Setting view and dialog to current canvas.", 0);
    m_fullscreenDlg = m_tabs[tab].fullScreenDlg();
    m_fullscreenView = m_tabs[tab].fullscreenView();
    connect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
    COMMENT("Setting editor elements to current tab: undo stack, scene, rectangle, simulation controller, IC manager. Also connecting IC Manager and simulation controller signals and setting global IC manager. Updating ICs if changed.", 0);
    m_editor->selectWorkspace(&m_tabs[tab]);
    COMMENT("Connecting undo and redo functions to UI menu.", 0);
    addUndoRedoMenu(tab);
    COMMENT("Setting Panda and Dolphin file info.", 0);
    m_currentFile = m_tabs[tab].currentFile();
    m_dolphinFileName = m_tabs[tab].dolphinFileName();
    GlobalProperties::currentFile = m_currentFile.absoluteFilePath();
    setCurrentDir();
    updateICList();
    COMMENT("Setting selected tab as the current one.", 0);
    m_current_tab = tab;
    COMMENT("Connecting current tab to element editor menu in UI.", 0);
    ui->widgetElementEditor->setScene(m_tabs[tab].scene());
    COMMENT("Reinitialize simulation controller.", 0);
    m_editor->getSimulationController()->start();
    m_editor->setHandlingEvents(true);
}

void MainWindow::createNewWorkspace()
{
    COMMENT("Creating new workspace.", 0);
    if (!m_tabs.empty()) {
        disconnectTab();
    }
    COMMENT("Creating workspace elements used by the editor.", 0);
    m_editor->setupWorkspace();
    COMMENT("Creating the view and dialog elements used by the main window canvas.", 0);
    buildFullScreenDialog();
    COMMENT("Creating the tab structure.", 0);
    createNewTab();
    COMMENT("Setting dolphin name.", 0);
    m_dolphinFileName = "";
    COMMENT("Creating autosave file.", 0);
    createAutosaveFile();
    COMMENT("Creating undo and redu action menus.", 0);
    createUndoRedoMenus();
    COMMENT("Selecting the newly created tab.", 0);
    m_current_tab = m_tabs.size() - 1;
    ui->tabWidget_mainWindow->setCurrentIndex(m_current_tab);
    COMMENT("Connecting current tab to element editor menu in UI.", 0);
    ui->widgetElementEditor->setScene(m_tabs[m_current_tab].scene());
    COMMENT("(Re)initialize simulation controller.", 0);
    m_editor->getSimulationController()->start();
    m_editor->setHandlingEvents(true);
    COMMENT("Finished creating new workspace.", 0);
}

void MainWindow::selectTab(int tab)
{
    COMMENT("Selecting tab: " << QString::number(tab).toStdString(), 0);
    if (tab != m_current_tab) {
        disconnectTab();
        connectTab(tab);
        m_editor->clearSelection();
    }
    COMMENT("New tab selected. BD filename: " << m_dolphinFileName.toStdString(), 0);
>>>>>>> master:app/mainwindow.cpp
}
>>>>>>> 252ae85 (Completed translation to Portuguese):app/mainwindow.cpp



void MainWindow::lineEditTextChanged(const QString &text)
{
    ui->searchLayout->removeItem(ui->VSpacer);
    for (auto *item : qAsConst(searchItemWidgets)) {
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
        auto ics = ui->tabWidget->findChildren<Label *>("label_ic");
        QRegularExpression regex(QString(".*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        QRegularExpression regex2(QString("^label_.*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        auto searchResults = ui->tabWidget->findChildren<Label *>(regex2);
        auto allLabels = ui->tabWidget->findChildren<Label *>();
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
        for (auto *label : searchResults) {
            auto *item = new ListItemWidget(label->pixmap(), label->elementType(), label->auxData());
            if (!m_firstResult) {
                m_firstResult = item->getLabel();
            }
            ui->searchLayout->addWidget(item);
            searchItemWidgets.append(item);
        }
    }
    ui->searchLayout->addItem(ui->VSpacer);
}

void MainWindow::lineEditReturnPressed()
{
    if (m_firstResult) {
        m_firstResult->startDrag();
        ui->lineEdit->clear();
    }
}

void MainWindow::resizeEvent(QResizeEvent * /*event*/)
{
    if (m_editor) {
        m_editor->getScene()->setSceneRect(m_editor->getScene()->sceneRect().united(m_fullscreenView->rect()));
    }
}

<<<<<<< HEAD:app/MainWindow.cpp
=======
void MainWindow::on_actionReload_File_triggered()
{
    if (!m_currentFile.exists()) {
        return;
    } else {
        auto *undostack = m_tabs[m_current_tab].undoStack();
        if (!undostack->isClean()) {
            int ret = confirmSave(false);
            if (ret == QMessageBox::Yes) {
                if (!save(m_tabs[m_current_tab].currentFile().absoluteFilePath())) {
                    if (closeTabAnyway() == QMessageBox::No) {
                        return;
                    }
                }
            } else if (ret == QMessageBox::Cancel) {
                return;
            }
            m_editor->selectAll();
            m_editor->deleteAction();
            m_editor->getUndoStack()->clear();
            loadPandaFile(m_currentFile.absoluteFilePath());
        }
    }
}
>>>>>>> master:app/mainwindow.cpp


void MainWindow::actionGatesTriggered(bool checked)
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

bool MainWindow::exportToWaveFormFile(const QString &fname)
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
        setEnabled(true);
        QMessageBox::warning(this, tr("Error"), tr("<strong>Error while exporting to waveform file:</strong><br>%1").arg(e.what()));
        return false;
    }
    return true;
}

<<<<<<< HEAD:app/MainWindow.cpp


void MainWindow::actionZoomInTriggered()
=======
bool MainWindow::on_actionExport_to_Arduino_triggered()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Generate Arduino Code"), m_defaultDirectory, tr("Arduino file (*.ino)"));
    return exportToArduino(fname);
}

void MainWindow::on_actionZoom_in_triggered() const
>>>>>>> master:app/mainwindow.cpp
{
    m_fullscreenView->gvzoom()->zoomIn();
}

<<<<<<< HEAD:app/MainWindow.cpp
void MainWindow::actionZoomOutTriggered()
=======
void MainWindow::on_actionZoom_out_triggered() const
>>>>>>> master:app/mainwindow.cpp
{
    m_fullscreenView->gvzoom()->zoomOut();
}

<<<<<<< HEAD:app/MainWindow.cpp
void MainWindow::actionResetZoomTriggered()
=======
void MainWindow::on_actionReset_Zoom_triggered() const
>>>>>>> master:app/mainwindow.cpp
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
    const auto files = m_rfController->getRecentFiles();
    const int numRecentFiles = qMin(files.size(), static_cast<int>(RecentFilesController::MaxRecentFiles));
    if (numRecentFiles > 0) {
        ui->menuRecent_files->setEnabled(true);
    }
    for (int i = 0; i < numRecentFiles; ++i) {
        const QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActs[i]->setText(text);
        m_recentFileActs[i]->setData(files[i]);
        m_recentFileActs[i]->setVisible(true);
    }
    for (int i = numRecentFiles; i < RecentFilesController::MaxRecentFiles; ++i) {
        m_recentFileActs[i]->setVisible(false);
    }
}

<<<<<<< HEAD:app/MainWindow.cpp





=======
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
    QString pngFile = QFileDialog::getSaveFileName(this, tr("Export to Image"), m_defaultDirectory, tr("PNG files (*.png)"));
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
>>>>>>> master:app/mainwindow.cpp

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
    ui->widgetElementEditor->retranslateUi();

    auto items = ui->tabWidget->findChildren<ListItemWidget *>();
    for (auto *item : qAsConst(items)) {
        item->updateName();
    }
}

void MainWindow::loadTranslation(QString language)
{
    if (language.isEmpty()) {
        return;
    }

    if (m_translator) {
        qApp->removeTranslator(m_translator);
    }

    if (language == "://wpanda_pt.qm") {
        language = "://wpanda_pt_BR.qm";
    }

    if (language == "://wpanda_en.qm") {
        return retranslateUi();
    }

    m_translator = new QTranslator(this);

    if (!m_translator->load(language)) {
        QMessageBox::critical(this, "Error!", "Error loading translation!");
        return;
    }

    qApp->installTranslator(m_translator);
    retranslateUi();
}

void MainWindow::actionEnglishTriggered()
{
    QString language = "://wpanda_en.qm";
    Settings::setValue("language", language);
    loadTranslation(language);
}

void MainWindow::actionPortugueseTriggered()
{
    QString language = "://wpanda_pt_BR.qm";
    Settings::setValue("language", language);
    loadTranslation(language);
}

void MainWindow::actionPlayTriggered(bool checked)
{
    if (checked) {
        m_editor->getSimulationController()->start();
    } else {
        m_editor->getSimulationController()->stop();
    }
    m_editor->getSimulationController()->updateAll();
}

void MainWindow::actionRenameTriggered()
{
    m_editor->getElementEditor()->renameAction();
}

void MainWindow::actionChangeTriggerTriggered()
{
    m_editor->getElementEditor()->changeTriggerAction();
}

void MainWindow::actionClearSelectionTriggered()
{
    if (m_fullscreenDlg->isVisible() && m_editor->getScene()->selectedItems().isEmpty()) {
        m_fullscreenDlg->accept();
    }
    m_editor->getScene()->clearSelection();
}

void MainWindow::populateMenu(QSpacerItem *spacer, const QString &names, QLayout *layout) // TODO: call this function with the list of files and set directory watcher.
{
    auto list(names.split(", "));
    layout->removeItem(spacer);
    for (auto &name : list) {
        name = name.trimmed().toUpper();
        auto type = ElementFactory::textToType(name);
        auto pixmap(ElementFactory::getPixmap(type));
        auto *item = new ListItemWidget(pixmap, type, name, this);
        layout->addWidget(item);
    }
    layout->addItem(spacer);
}

void MainWindow::populateLeftMenu()
{
    ui->tabWidget->setCurrentIndex(0);
    populateMenu(ui->verticalSpacer_InOut, "VCC, GND, BUTTON, SWITCH, ROTARY, CLOCK, LED, DISPLAY, DISPLAY14, BUZZER", ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(ui->verticalSpacer_Gates, "AND, OR, NOT, NAND, NOR, XOR, XNOR, MUX, DEMUX, NODE", ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(ui->verticalSpacer_Memory, "DFLIPFLOP, DLATCH, JKFLIPFLOP, SRFLIPFLOP, TFLIPFLOP", ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(ui->verticalSpacer_MISC, "TEXT, LINE", ui->scrollAreaWidgetContents_Misc->layout());
}

void MainWindow::actionFastModeTriggered(bool checked)
{
    setFastMode(checked);
    Settings::setValue("fastMode", checked);
}

void MainWindow::actionWaveFormTriggered()
{
    if (m_bd == nullptr) {
        m_bd = new BewavedDolphin(m_editor, this);
    }
    if (m_bd->createWaveform(m_dolphinFileName)) {
        COMMENT("BD filename: " << m_dolphinFileName.toStdString(), 0);
        m_bd->show();
    } else {
        setEnabled(true);
    }
}

void MainWindow::actionPandaLightTriggered()
{
    ThemeManager::globalManager->setTheme(Theme::Light);
}

void MainWindow::actionPandaDarkTriggered()
{
    ThemeManager::globalManager->setTheme(Theme::Dark);
}

void MainWindow::updateTheme()
{
    switch (ThemeManager::globalManager->theme()) {
    case Theme::Dark:  ui->actionPanda_Dark->setChecked(true); break;
    case Theme::Light: ui->actionPanda_Light->setChecked(true); break;
    }
}

void MainWindow::actionFlipHorizontallyTriggered()
{
    m_editor->flipH();
}

void MainWindow::actionFlipVerticallyTriggered()
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
    dlg_layout->addWidget(m_fullscreenView);
    m_fullscreenDlg->setLayout(dlg_layout);
    m_fullscreenDlg->setStyleSheet("QGraphicsView { border-style: none; }");
    m_fullscreenView->setScene(m_editor->getScene());
    connect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
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

void MainWindow::actionFullScreenTriggered() const
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
    QString allAutoSaveFileNames;
    COMMENT("Cheking if autosavefile exists and if it contains current project file. If so, remove autosavefile from it.", 0);
    if (Settings::contains("autosaveFile")) {
        allAutoSaveFileNames = Settings::value("autosaveFile").toString();
        COMMENT("All auto save file names before autosaving: " << allAutoSaveFileNames.toStdString(), 3);
        if ((!m_autoSaveFile[m_current_tab]->fileName().isEmpty()) && (allAutoSaveFileNames.contains(m_autoSaveFile[m_current_tab]->fileName()))) {
            COMMENT("Removing current autosave file name.", 3);
            allAutoSaveFileNames.remove(m_autoSaveFile[m_current_tab]->fileName() + "\t");
            Settings::setValue("autosaveFile", allAutoSaveFileNames);
        }
    }
    COMMENT("All auto save file names after possibly removing autosave: " << allAutoSaveFileNames.toStdString(), 0);
    COMMENT("If autosave exists and undo stack is clean, remove it.", 0);
    auto *undostack = m_tabs[m_current_tab].undoStack();
    COMMENT("undostack element: " << undostack->index() << " of " << undostack->count(), 0);
    if (undostack->isClean()) {
        if (m_currentFile.exists()) {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, m_currentFile.fileName());
        } else {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, tr("New Project"));
        }
        COMMENT("Undo stack is clean.", 3);
        if (m_autoSaveFile[m_current_tab]->exists()) {
            m_autoSaveFile[m_current_tab]->remove();
        }
    } else {
        COMMENT("Undo in not clean. Must set autosave file.", 3);
        if (m_currentFile.exists()) {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, m_currentFile.fileName() + "*");
        } else {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, tr("New Project*"));
        }
        if (m_autoSaveFile[m_current_tab]->exists()) {
            COMMENT("Autosave file already exists. Delete it to update.", 3);
            m_autoSaveFile[m_current_tab]->remove();
            if (!m_currentFile.fileName().isEmpty()) {
                QDir autosavePath(QDir::temp());
                COMMENT("Autosave path set to the current file's directory, if there is one.", 3);
                autosavePath = m_currentFile.dir();
                COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 3);
                m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath("." + m_currentFile.baseName() + "XXXXXX.panda"));
                COMMENT("Setting current file to: " << m_currentFile.absoluteFilePath().toStdString(), 3);
            } else {
                COMMENT("Default value not set yet.", 3);
                QDir autosavePath(QDir::temp());
                COMMENT("Autosavepath: " << autosavePath.absolutePath().toStdString(), 3);
                m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
                COMMENT("Setting current file to random file in tmp.", 3);
            }
        }
        if (m_autoSaveFile[m_current_tab]->open()) {
            COMMENT("Writing to autosave file.", 3);
            QDataStream ds(m_autoSaveFile[m_current_tab]);
            QString autosaveFilename = m_autoSaveFile[m_current_tab]->fileName() + "\t";
            try {
                m_editor->save(ds, m_dolphinFileName);
                allAutoSaveFileNames.append(autosaveFilename);
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
                COMMENT("All auto save file names after adding autosave: " << allAutoSaveFileNames.toStdString(), 3);
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

void MainWindow::actionMuteTriggered()
{
    m_editor->mute(ui->actionMute->isChecked());
    if (ui->actionMute->isChecked()) {
        ui->actionMute->setText(tr("Unmute"));
    } else {
        ui->actionMute->setText(tr("Mute"));
    }
}

void MainWindow::actionLabelsUnderIconsTriggered(bool checked)
{
    checked ? ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon) : ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
}
<<<<<<< HEAD:app/MainWindow.cpp


=======
>>>>>>> master:app/mainwindow.cpp
