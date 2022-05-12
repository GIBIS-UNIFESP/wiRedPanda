// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "bewaveddolphin.h"
#include "codegenerator.h"
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

#include <QActionGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QLoggingCategory>
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

MainWindow::MainWindow(QWidget *parent, const QString &filename)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_bd(nullptr)
    , m_editor(new Editor(this))
    , m_firstResult(nullptr)
    , m_dolphinFileName("")
    , m_autoSaveFile(nullptr)
    , m_pandaTranslator(nullptr)
    , m_loadedAutoSave(false)
{
    qCDebug(zero) << "WiRedPanda Version =" << APP_VERSION << "OR" << GlobalProperties::version;

    ui->setupUi(this);
    ThemeManager::globalManager = new ThemeManager(this);

    qCDebug(zero) << "Settings filename:" << Settings::fileName();
    loadTranslation(Settings::value("language").toString());

    qCDebug(zero) << "Building dialog within a new tab.";
    m_current_tab = 0;
    createNewWorkspace();
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabBarClicked, this, &MainWindow::selectTab);
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    qCDebug(zero) << "Restoring geometry and setting zoom controls.";
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

    qCDebug(zero) << "Preparing theme and UI modes.";
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

    qCDebug(zero) << "Setting left side menus.";
    m_editor->setElementEditor(ui->widgetElementEditor);
    ui->searchScrollArea->hide();
    auto *shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(shortcut, &QShortcut::activated, ui->lineEdit, qOverload<>(&QWidget::setFocus));
    populateLeftMenu();

    qCDebug(zero) << "Setting directories and autosave file.";
    setCurrentFile({});

    qCDebug(zero) << "Creating edit menu funcionalities.";
    connect(ui->actionCopy, &QAction::triggered, m_editor, &Editor::copyAction);
    connect(ui->actionCut, &QAction::triggered, m_editor, &Editor::cutAction);
    connect(ui->actionPaste, &QAction::triggered, m_editor, &Editor::pasteAction);
    connect(ui->actionDelete, &QAction::triggered, m_editor, &Editor::deleteAction);
    connect(ui->actionAbout_this_version, &QAction::triggered, this, &MainWindow::aboutThisVersion);
    ui->actionWaveform->setDisabled(true);

    qCDebug(zero) << "Setting up zoom and screen funcionalities.";
    connect(m_editor, &Editor::scroll, this, &MainWindow::scrollView);
    connect(m_editor, &Editor::circuitHasChanged, this, &MainWindow::autoSave);
    connect(m_editor, &Editor::circuitAppearenceHasChanged, this, &MainWindow::autoSave);
    m_fullscreenView->setCacheMode(QGraphicsView::CacheBackground);

    qCDebug(zero) << "Loading recent file list.";
    m_rfController = new RecentFilesController("recentFileList", this, true);
    connect(this, &MainWindow::addRecentFile, m_rfController, &RecentFilesController::addRecentFile);
    m_firstResult = nullptr;
    // updateRecentICs();
    createRecentFileActions();
    connect(m_rfController, &RecentFilesController::recentFilesUpdated, this, &MainWindow::updateRecentFileActions);

    qCDebug(zero) << "Checking for autosave file recovery.";
    loadAutoSaveFiles(filename);
    qCDebug(zero) << "Checking playing simulation.";
    ui->actionPlay->setChecked(true);

    qCDebug(zero) << "Connecting change tab shortcut to tab selection function.";
    auto *nextTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), ui->tabWidget_mainWindow);
    connect(nextTabShortcut, &QShortcut::activated, this, &MainWindow::selectNextTab);
    auto *prevTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backtab), ui->tabWidget_mainWindow);
    connect(prevTabShortcut, &QShortcut::activated, this, &MainWindow::selectPreviousTab);

    qCDebug(zero) << "Setting connections";
    connect(ui->actionAbout_Qt, &QAction::triggered, this, &MainWindow::on_actionAbout_Qt_triggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::on_actionAbout_triggered);
    connect(ui->actionChange_Trigger, &QAction::triggered, this, &MainWindow::on_actionChange_Trigger_triggered);
    connect(ui->actionEnglish, &QAction::triggered, this, &MainWindow::on_actionEnglish_triggered);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::on_actionExit_triggered);
    connect(ui->actionExport_to_Image, &QAction::triggered, this, &MainWindow::on_actionExport_to_Image_triggered);
    connect(ui->actionFast_Mode, &QAction::triggered, this, &MainWindow::on_actionFast_Mode_triggered);
    connect(ui->actionFlip_horizontally, &QAction::triggered, this, &MainWindow::on_actionFlip_horizontally_triggered);
    connect(ui->actionFlip_vertically, &QAction::triggered, this, &MainWindow::on_actionFlip_vertically_triggered);
    connect(ui->actionFullscreen, &QAction::triggered, this, &MainWindow::on_actionFullscreen_triggered);
    connect(ui->actionGates, &QAction::triggered, this, &MainWindow::on_actionGates_triggered);
    connect(ui->actionLabels_under_icons, &QAction::triggered, this, &MainWindow::on_actionLabels_under_icons_triggered);
    connect(ui->actionMute, &QAction::triggered, this, &MainWindow::on_actionMute_triggered);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::on_actionNew_triggered);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);
    connect(ui->actionPanda_Dark, &QAction::triggered, this, &MainWindow::on_actionPanda_Dark_triggered);
    connect(ui->actionPanda_Light, &QAction::triggered, this, &MainWindow::on_actionPanda_Light_triggered);
    connect(ui->actionPlay, &QAction::triggered, this, &MainWindow::on_actionPlay_triggered);
    connect(ui->actionPortuguese, &QAction::triggered, this, &MainWindow::on_actionPortuguese_triggered);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::on_actionPrint_triggered);
    connect(ui->actionReload_File, &QAction::triggered, this, &MainWindow::on_actionReload_File_triggered);
    connect(ui->actionRename, &QAction::triggered, this, &MainWindow::on_actionRename_triggered);
    connect(ui->actionReset_Zoom, &QAction::triggered, this, &MainWindow::on_actionReset_Zoom_triggered);
    connect(ui->actionRotate_left, &QAction::triggered, this, &MainWindow::on_actionRotate_left_triggered);
    connect(ui->actionRotate_right, &QAction::triggered, this, &MainWindow::on_actionRotate_right_triggered);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::on_actionSave_As_triggered);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::on_actionSave_triggered);
    connect(ui->actionSelect_all, &QAction::triggered, this, &MainWindow::on_actionSelect_all_triggered);
    connect(ui->actionWaveform, &QAction::triggered, this, &MainWindow::on_actionWaveform_triggered);
    connect(ui->actionWires, &QAction::triggered, this, &MainWindow::on_actionWires_triggered);
    connect(ui->actionZoom_in, &QAction::triggered, this, &MainWindow::on_actionZoom_in_triggered);
    connect(ui->actionZoom_out, &QAction::triggered, this, &MainWindow::on_actionZoom_out_triggered);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::on_lineEdit_returnPressed);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_textChanged);

    qCDebug(zero) << "Window title.";
    setWindowTitle("WiRedPanda " + QString(APP_VERSION));
}

void MainWindow::selectNextTab()
{
    qCDebug(zero) << "Forward:" << m_current_tab;
    selectTab((m_current_tab + 1) % m_tabs.size());
    ui->tabWidget_mainWindow->setCurrentIndex(m_current_tab);
    qCDebug(zero) << "Forward end:" << m_current_tab;
}

void MainWindow::selectPreviousTab()
{
    qCDebug(zero) << "Backwards:" << m_current_tab;
    selectTab((m_current_tab + m_tabs.size() - 1) % m_tabs.size());
    ui->tabWidget_mainWindow->setCurrentIndex(m_current_tab);
    qCDebug(zero) << "Backwards end:" << m_current_tab;
}

void MainWindow::loadAutoSaveFiles(const QString &filename)
{
    if (Settings::contains("autosaveFile")) {
        qCDebug(zero) << "autosave files exist in config file.";
        bool yes_to_all = false;
        bool no_to_all = false;
        QString allAutoSaveFileNames(Settings::value("autosaveFile").toString());
        QStringList autoSaveFileNameList(allAutoSaveFileNames.split("\t", SKIPEMPTYPARTS));
        qCDebug(zero) << "all files:" << allAutoSaveFileNames;
        for (const auto &autoSaveFileName : qAsConst(autoSaveFileNameList)) {
            if ((QFile(autoSaveFileName).exists()) && (autoSaveFileName != filename)) {
                qCDebug(zero) << "Autosave exists and was not open. Trying to recover.";
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
                qCDebug(zero) << "Loading autosave.";
                m_loadedAutoSave = true;
            } else if (!QFile(autoSaveFileName).exists()) {
                qCDebug(zero) << "Removing autosave file name from config that does not exist.";
                allAutoSaveFileNames.remove(autoSaveFileName + "\t");
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
            }
        }
    } else {
        qCDebug(zero) << "autosave files do no exist in config file.";
    }
}

void MainWindow::createNewTab()
{
    qCDebug(zero) << "Dialog built. Now adding tab. #tabs:" << m_tabs.size() << ", current tab:" << m_current_tab;
    auto *new_tab = new QWidget(ui->tabWidget_mainWindow);
    new_tab->setLayout(m_fullscreenDlg->layout());
    new_tab->layout()->addWidget(m_fullscreenView);
    ui->tabWidget_mainWindow->addTab(new_tab, "");
    qCDebug(zero) << "Storing tab pointers.";
    m_tabs.push_back(WorkSpace(m_fullscreenDlg, m_fullscreenView, m_editor));
    qCDebug(zero) << "Finished #tabs:" << m_tabs.size() << ", current tab:" << m_current_tab;
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
}

void MainWindow::setFastMode(bool fastModeEnabled)
{
    m_fullscreenView->setRenderHint(QPainter::Antialiasing, !fastModeEnabled);
    m_fullscreenView->setRenderHint(QPainter::TextAntialiasing, !fastModeEnabled);
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
    qCDebug(zero) << "fname:" << fname;
    qCDebug(zero) << "Getting autosave settings info.";
    QString allAutoSaveFileNames;
    if (Settings::contains("autosaveFile")) {
        allAutoSaveFileNames = Settings::value("autosaveFile").toString();
    }
    qCDebug(zero) << "All auto save file names before save:" << allAutoSaveFileNames;
    qCDebug(zero) << "Checking if it is an autosave file or new a new project, and ask for a filename.";
    QString autoSaveFileName;
    if ((fname.isEmpty()) || ((!allAutoSaveFileNames.isEmpty()) && (allAutoSaveFileNames.contains(fname)))) {
        qCDebug(zero) << "Should open window.";
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
            qCDebug(zero) << "Error saving project:" << e.what();
            return false;
        }
    }
    if (fl.commit()) {
        setCurrentFile(fname);
        ui->statusBar->showMessage(tr("Saved file successfully."), 4000);
        m_editor->getUndoStack()->setClean();
        qCDebug(zero) << "Remove from autosave list recovered file that has been saved.";
        if (!autoSaveFileName.isEmpty()) {
            allAutoSaveFileNames.remove(autoSaveFileName + "\t");
            Settings::setValue("autosaveFile", allAutoSaveFileNames);
            qCDebug(zero) << "All auto save file names after removing recovered:" << allAutoSaveFileNames;
        }
        qCDebug(zero) << "Remove autosave from settings and deleting it.";
        if (m_autoSaveFile[m_current_tab]->isOpen()) {
            allAutoSaveFileNames.remove(m_autoSaveFile[m_current_tab]->fileName() + "\t");
            Settings::setValue("autosaveFile", allAutoSaveFileNames + "\n");
            m_autoSaveFile[m_current_tab]->remove();
            qCDebug(zero) << "All auto save file names after removing autosave:" << allAutoSaveFileNames;
        }
        return true;
    }
    qCDebug(zero) << "Could not save file:" << fl.errorString();
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
    qCDebug(zero) << "'What is new' message box.";
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
    msgBox.setText(tr("File not saved. Close tab anyway?"));
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}


int MainWindow::recoverAutoSaveFile(const QString &autosaveFilename)
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);
    msgBox.setText(tr("An autosave file was found. Do you want to load it?\n") + autosaveFilename);
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
    msgBox.setText(tr("Error while opening autosave file. Do you want to delete it ?\n Autosave: ") + autosaveFilename);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::No);
    return msgBox.exec();
}

void MainWindow::on_actionNew_triggered()
{
    createNewWorkspace();
    setCurrentFile({});
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
        QMessageBox::critical(this, tr("Error!"), tr("File \"%1\" does not exist!").arg(fname), QMessageBox::Ok, QMessageBox::NoButton);
        qCDebug(zero) << "Error: This file does not exist:" << fname;
        return false;
    }
    qCDebug(zero) << "File exists";
    if (fl.open(QFile::ReadOnly)) {
        qCDebug(zero) << "File opened:" << fname;
        const int current_tab = m_current_tab; // Stored to possibly recover it in case of error.
        createNewWorkspace();
        try {
            qCDebug(zero) << "Current file set.";
            setCurrentFile(fname);
            qCDebug(zero) << "Loading in editor.";
            QDataStream ds(&fl);
            m_editor->load(ds);
            updateICList();
        } catch (std::runtime_error &e) {
            qCDebug(zero) << "Error loading project:" << e.what();
            QMessageBox::critical(this, tr("Error!"), tr("Could not open file.\nError: %1").arg(e.what()), QMessageBox::Ok, QMessageBox::NoButton);
            closeTab(m_tabs.size() - 1);
            selectTab(current_tab);
            return false;
        }
    } else {
        qCDebug(zero) << "Could not open file in ReadOnly mode:" << fname;
        return false;
    }
    qCDebug(zero) << "Closing file.";
    fl.close();
    ui->statusBar->showMessage(tr("File loaded successfully."), 2000);
    return true;
}

void MainWindow::scrollView(int dx, int dy) const
{
    m_fullscreenView->scroll(dx, dy);
}

void MainWindow::on_actionOpen_triggered()
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

void MainWindow::on_actionAbout_Qt_triggered()
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
    for (auto tab : qAsConst(m_tabs)) {
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
        qCDebug(zero) << "Adding file to controller.";
        emit addRecentFile(file.absoluteFilePath());
    }
    qCDebug(zero) << "Setting global current file and dir.";
    GlobalProperties::currentFile = m_currentFile.absoluteFilePath();
    setCurrentDir();
}

void MainWindow::setAutoSaveFileName(const QFileInfo &file)
{
    qCDebug(zero) << "Defining autosave path.";
    if (file.exists()) {
        QDir autosavePath(QDir::temp());
        qCDebug(zero) << "Autosave path set to the current file's directory, if there is one.";
        autosavePath = file.dir();
        qCDebug(zero) << "Autosavepath:" << autosavePath.absolutePath();
        m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath("." + file.baseName() + "XXXXXX.panda"));
        qCDebug(zero) << "Setting current file to:" << file.absoluteFilePath();
    } else {
        qCDebug(zero) << "Default file does not exist:" << file.absoluteFilePath();
        QDir autosavePath(QDir::temp());
        qCDebug(zero) << "Autosavepath:" << autosavePath.absolutePath();
        m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
        qCDebug(zero) << "Setting current file to random file in tmp.";
    }
}

void MainWindow::on_actionSelect_all_triggered()
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
            ui->scrollAreaWidgetContents_Box->layout()->addWidget(item);
        }
    }
    ui->scrollAreaWidgetContents_Box->layout()->addItem(ui->verticalSpacer_IC);
}

void MainWindow::closeTab(int tab)
{
    if (m_tabs.size() == 1) {
        close();
    } else {
        closeTabAction(tab);
    }
}

bool MainWindow::closeTabAction(int tab)
{
    qCDebug(zero) << "Closing tab" << tab << ", #tabs:" << m_tabs.size() << ", current tab:" << m_current_tab;
    QString allAutoSaveFileNames;
    QString autoSaveFileName;
    qCDebug(zero) << "Reading autosave file names.";
    if (Settings::contains("autosaveFile")) {
        allAutoSaveFileNames = Settings::value("autosaveFile").toString();
        qCDebug(zero) << "Verifying if this is a recovered autosave file.";
        if (allAutoSaveFileNames.contains(m_tabs[tab].currentFile().fileName())) {
            autoSaveFileName = m_tabs[tab].currentFile().absoluteFilePath();
        }
    }
    qCDebug(zero) << "All auto save file names before closing tab:" << allAutoSaveFileNames;
    qCDebug(zero) << "Checking if needs to save file.";
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
                qCDebug(zero) << "Discarding recovered autosave file.";
                allAutoSaveFileNames.remove(autoSaveFileName + "\t");
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
                qCDebug(zero) << "All auto save file names after removing recovery file:" << allAutoSaveFileNames;
            }
            if (!m_tabs[tab].undoStack()->isClean()) {
                qCDebug(zero) << "Discarding autosave modification.";
                allAutoSaveFileNames.remove(m_autoSaveFile[tab]->fileName() + "\t");
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
                m_autoSaveFile[tab]->remove(); // This only removes the autosave file itself, not the QTempFile object.
                qCDebug(zero) << "All auto save file names after removing autosave file:" << allAutoSaveFileNames;
            }
        }
    }
    qCDebug(zero) << "Moving to other tab if closed tab is the current one and not the only one. Othewise, if has just one tab, disconnects it from the UI.";
    if ((tab == m_current_tab) && (m_tabs.size() != 1)) {
        selectTab((tab + 1) % m_tabs.size());
    } else if (m_tabs.size() == 1) {
        disconnectTab();
    }
    qCDebug(zero) << "Deleting tab and autosave.";
    removeAutosaveFile(tab); // This removes the QTempFile object allocation.
    m_tabs[tab].free();
    m_tabs.remove(tab);
    m_undoAction.remove(tab);
    m_redoAction.remove(tab);
    ui->tabWidget_mainWindow->removeTab(tab);
    qCDebug(zero) << "Adjusting m_current_tab if removed tab lays before the current one.";
    if (m_current_tab > tab) {
        m_current_tab--;
    }
    qCDebug(zero) << "Closed tab" << tab << ", #tabs:" << m_tabs.size() << ", current tab:" << m_current_tab;
    return true;
}

void MainWindow::disconnectTab()
{
    qCDebug(zero) << "Stopping simulation controller and event handling.";
    m_editor->setHandlingEvents(false);
    m_editor->getSimulationController()->stop();
    qCDebug(zero) << "Disconnecting zoom from UI controllers.";
    disconnect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
    qCDebug(zero) << "Removing undo and redo actions from UI menu.";
    removeUndoRedoMenu();
    qCDebug(zero) << "Disabling beWavedDolphin action.";
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
    qCDebug(zero) << "Setting view and dialog to current canvas.";
    m_fullscreenDlg = m_tabs[tab].fullScreenDlg();
    m_fullscreenView = m_tabs[tab].fullscreenView();
    connect(m_fullscreenView->gvzoom(), &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged);
    qCDebug(zero) << "Setting editor elements to current tab: undo stack, scene, rectangle, simulation controller, IC manager. Also connecting IC Manager and simulation controller signals and setting global IC manager. Updating ICs if changed.";
    m_editor->selectWorkspace(&m_tabs[tab]);
    qCDebug(zero) << "Connecting undo and redo functions to UI menu.";
    addUndoRedoMenu(tab);
    qCDebug(zero) << "Setting Panda and Dolphin file info.";
    m_currentFile = m_tabs[tab].currentFile();
    m_dolphinFileName = m_tabs[tab].dolphinFileName();
    GlobalProperties::currentFile = m_currentFile.absoluteFilePath();
    setCurrentDir();
    updateICList();
    qCDebug(zero) << "Setting selected tab as the current one.";
    m_current_tab = tab;
    qCDebug(zero) << "Connecting current tab to element editor menu in UI.";
    ui->widgetElementEditor->setScene(m_tabs[tab].scene());
    qCDebug(zero) << "Reinitialize simulation controller.";
    m_editor->getSimulationController()->start();
    m_editor->setHandlingEvents(true);
}

void MainWindow::createNewWorkspace()
{
    qCDebug(zero) << "Creating new workspace.";
    if (!m_tabs.empty()) {
        disconnectTab();
    }
    qCDebug(zero) << "Creating workspace elements used by the editor.";
    m_editor->setupWorkspace();
    qCDebug(zero) << "Creating the view and dialog elements used by the main window canvas.";
    buildFullScreenDialog();
    qCDebug(zero) << "Creating the tab structure.";
    createNewTab();
    qCDebug(zero) << "Setting dolphin name.";
    m_dolphinFileName = "";
    qCDebug(zero) << "Creating autosave file.";
    createAutosaveFile();
    qCDebug(zero) << "Creating undo and redu action menus.";
    createUndoRedoMenus();
    qCDebug(zero) << "Selecting the newly created tab.";
    m_current_tab = m_tabs.size() - 1;
    ui->tabWidget_mainWindow->setCurrentIndex(m_current_tab);
    qCDebug(zero) << "Connecting current tab to element editor menu in UI.";
    ui->widgetElementEditor->setScene(m_tabs[m_current_tab].scene());
    qCDebug(zero) << "(Re)initialize simulation controller.";
    m_editor->getSimulationController()->start();
    m_editor->setHandlingEvents(true);
    qCDebug(zero) << "Finished creating new workspace.";
}

void MainWindow::selectTab(int tab)
{
    qCDebug(zero) << "Selecting tab:" << tab;
    if (tab != m_current_tab) {
        disconnectTab();
        connectTab(tab);
        m_editor->clearSelection();
    }
    qCDebug(zero) << "New tab selected. BD filename:" << m_dolphinFileName;
}

void MainWindow::on_lineEdit_textChanged(const QString &text)
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
        for (auto *label : qAsConst(searchResults)) {
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

void MainWindow::on_lineEdit_returnPressed()
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

        qCDebug(zero) << "Arduino code successfully generated.";
    } catch (std::runtime_error &e) {
        QMessageBox::critical(this, tr("Error!"), tr("<strong>Error while exporting to Arduino code:</strong><br>%1").arg(e.what()));
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
        m_bd = new BewavedDolphin(m_editor, this, false);
        if (!m_bd->createWaveform(fname)) {
            qCDebug(zero) << "Could not open waveform file:" << m_currentFile.fileName();
            delete m_bd;
            return false;
        }
        m_bd->print();
    } catch (std::runtime_error &e) {
        setEnabled(true);
        QMessageBox::critical(this, tr("Error!"), tr("<strong>Error while exporting to waveform file:</strong><br>%1").arg(e.what()));
        return false;
    }
    return true;
}

bool MainWindow::on_actionExport_to_Arduino_triggered()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Generate Arduino Code"), m_defaultDirectory, tr("Arduino file (*.ino)"));
    return exportToArduino(fname);
}

void MainWindow::on_actionZoom_in_triggered() const
{
    m_fullscreenView->gvzoom()->zoomIn();
}

void MainWindow::on_actionZoom_out_triggered() const
{
    m_fullscreenView->gvzoom()->zoomOut();
}

void MainWindow::on_actionReset_Zoom_triggered() const
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
        const QString text = "&" + QString::number(i + 1) + " " + QFileInfo(files[i]).fileName();
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
    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export to PDF"), m_defaultDirectory, tr("PDF files (*.pdf)"));
    if (pdfFile.isEmpty()) {
        return;
    }
    if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
        pdfFile.append(".pdf");
    }
    // TODO: replace QPrinter with QPdfWriter?
    // use QPainter::LosslessImageRendering
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFile);
    QPainter p;
    if (!p.begin(&printer)) {
        QMessageBox::critical(this, tr("Error!"), tr("Could not print this circuit to PDF."), QMessageBox::Ok);
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

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
    ui->widgetElementEditor->retranslateUi();

    auto items = ui->tabWidget->findChildren<ListItemWidget *>();
    for (auto *item : qAsConst(items)) {
        item->updateName();
    }
}

void MainWindow::loadTranslation(const QString &language)
{
    if (language.isEmpty()) {
        return;
    }

    if (m_pandaTranslator) {
        qApp->removeTranslator(m_pandaTranslator);
    }

    if (m_qtTranslator) {
        qApp->removeTranslator(m_qtTranslator);
    }

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
        QMessageBox::critical(this, "Error!", "Error loading WiRedPanda translation!");
        return;
    }

    m_qtTranslator = new QTranslator(this);

    if (!m_qtTranslator->load(qtFile) || !qApp->installTranslator(m_qtTranslator)) {
        QMessageBox::critical(this, "Error!", "Error loading Qt translation!");
        return;
    }

    retranslateUi();
}

void MainWindow::on_actionEnglish_triggered()
{
    QString language = "://wpanda_en.qm";
    Settings::setValue("language", language);
    loadTranslation(language);
}

void MainWindow::on_actionPortuguese_triggered()
{
    QString language = "://wpanda_pt_BR.qm";
    Settings::setValue("language", language);
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

void MainWindow::populateMenu(QSpacerItem *spacer, const QString &names, QLayout *layout) // TODO: call this function with the list of files and set directory watcher.
{
    auto list(names.split(", "));
    layout->removeItem(spacer);
    for (auto &name : list) {
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
    populateMenu(ui->verticalSpacer_InOut, "InputVcc, InputGnd, InputButton, InputSwitch, InputRotary, Clock, Led, Display, Display14, Buzzer", ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(ui->verticalSpacer_Gates, "And, Or, Not, Nand, Nor, Xor, Xnor, Mux, Demux, Node", ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(ui->verticalSpacer_Memory, "DFlipFlop, DLatch, JKFlipFlop, SRFlipFlop, TFlipFlop", ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(ui->verticalSpacer_MISC, "Text, Line", ui->scrollAreaWidgetContents_Misc->layout());
}

void MainWindow::on_actionFast_Mode_triggered(bool checked)
{
    setFastMode(checked);
    Settings::setValue("fastMode", checked);
}

void MainWindow::on_actionWaveform_triggered()
{
    if (m_bd == nullptr) {
        m_bd = new BewavedDolphin(m_editor, this);
    }
    if (m_bd->createWaveform(m_dolphinFileName)) {
        qCDebug(zero) << "BD filename:" << m_dolphinFileName;
        m_bd->show();
    } else {
        setEnabled(true);
    }
}

void MainWindow::on_actionPanda_Light_triggered()
{
    ThemeManager::globalManager->setTheme(Theme::Light);
}

void MainWindow::on_actionPanda_Dark_triggered()
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
    dlg_layout->addWidget(m_fullscreenView);
    m_fullscreenDlg->setLayout(dlg_layout);
    m_fullscreenDlg->setStyleSheet("QGraphicsView { border-style: none; }");
    m_fullscreenView->setScene(m_editor->getScene());
    const bool fastModeEnabled = ui->actionFast_Mode->isChecked();
    m_fullscreenView->setRenderHint(QPainter::Antialiasing, !fastModeEnabled);
    m_fullscreenView->setRenderHint(QPainter::TextAntialiasing, !fastModeEnabled);
    m_fullscreenView->setRenderHint(QPainter::SmoothPixmapTransform, !fastModeEnabled);
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

void MainWindow::on_actionFullscreen_triggered()
{
    isFullScreen() ? showNormal() : showFullScreen();
}

void MainWindow::autoSave()
{
    qCDebug(two) << "Starting autosave.";
    QString allAutoSaveFileNames;
    qCDebug(zero) << "Cheking if autosavefile exists and if it contains current project file. If so, remove autosavefile from it.";
    if (Settings::contains("autosaveFile")) {
        allAutoSaveFileNames = Settings::value("autosaveFile").toString();
        qCDebug(three) << "All auto save file names before autosaving:" << allAutoSaveFileNames;
        if ((!m_autoSaveFile[m_current_tab]->fileName().isEmpty()) && (allAutoSaveFileNames.contains(m_autoSaveFile[m_current_tab]->fileName()))) {
            qCDebug(three) << "Removing current autosave file name.";
            allAutoSaveFileNames.remove(m_autoSaveFile[m_current_tab]->fileName() + "\t");
            Settings::setValue("autosaveFile", allAutoSaveFileNames);
        }
    }
    qCDebug(zero) << "All auto save file names after possibly removing autosave:" << allAutoSaveFileNames;
    qCDebug(zero) << "If autosave exists and undo stack is clean, remove it.";
    auto *undostack = m_tabs[m_current_tab].undoStack();
    qCDebug(zero) << "undostack element:" << undostack->index() << "of" << undostack->count();
    if (undostack->isClean()) {
        if (m_currentFile.exists()) {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, m_currentFile.fileName());
        } else {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, tr("New Project"));
        }
        qCDebug(three) << "Undo stack is clean.";
        if (m_autoSaveFile[m_current_tab]->exists()) {
            m_autoSaveFile[m_current_tab]->remove();
        }
    } else {
        qCDebug(three) << "Undo is not clean. Must set autosave file.";
        if (m_currentFile.exists()) {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, m_currentFile.fileName() + "*");
        } else {
            ui->tabWidget_mainWindow->setTabText(m_current_tab, tr("New Project*"));
        }
        if (m_autoSaveFile[m_current_tab]->exists()) {
            qCDebug(three) << "Autosave file already exists. Delete it to update.";
            m_autoSaveFile[m_current_tab]->remove();
            if (!m_currentFile.fileName().isEmpty()) {
                QDir autosavePath(QDir::temp());
                qCDebug(three) << "Autosave path set to the current file's directory, if there is one.";
                autosavePath = m_currentFile.dir();
                qCDebug(three) << "Autosavepath:" << autosavePath.absolutePath();
                m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath("." + m_currentFile.baseName() + "XXXXXX.panda"));
                qCDebug(three) << "Setting current file to:" << m_currentFile.absoluteFilePath();
            } else {
                qCDebug(three) << "Default value not set yet.";
                QDir autosavePath(QDir::temp());
                qCDebug(three) << "Autosavepath:" << autosavePath.absolutePath();
                m_autoSaveFile[m_current_tab]->setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
                qCDebug(three) << "Setting current file to random file in tmp.";
            }
        }
        if (m_autoSaveFile[m_current_tab]->open()) {
            qCDebug(three) << "Writing to autosave file.";
            QDataStream ds(m_autoSaveFile[m_current_tab]);
            QString autosaveFilename = m_autoSaveFile[m_current_tab]->fileName() + "\t";
            try {
                m_editor->save(ds, m_dolphinFileName);
                allAutoSaveFileNames.append(autosaveFilename);
                Settings::setValue("autosaveFile", allAutoSaveFileNames);
                qCDebug(three) << "All auto save file names after adding autosave:" << allAutoSaveFileNames;
            } catch (std::runtime_error &e) {
                qCDebug(zero) << "Error autosaving project:" << e.what();
                m_autoSaveFile[m_current_tab]->close();
                m_autoSaveFile[m_current_tab]->remove();
            }
            m_autoSaveFile[m_current_tab]->close();
        }
    }
    qCDebug(three) << "Finished autosave.";
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
