// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "bewaveddolphin.h"
#include "codegenerator.h"
#include "common.h"
#include "dflipflop.h"
#include "elementfactory.h"
#include "elementlabel.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicsview.h"
#include "icmanager.h"
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

    connect(m_ui->tab, &QTabWidget::currentChanged,    this, &MainWindow::selectTab);
    connect(m_ui->tab, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    qCDebug(zero) << "Restoring geometry and setting zoom controls.";
    restoreGeometry(Settings::value("MainWindow/geometry").toByteArray());
    restoreState(Settings::value("MainWindow/windowState").toByteArray());
    m_ui->splitter->restoreGeometry(Settings::value("MainWindow/splitter/geometry").toByteArray());
    m_ui->splitter->restoreState(Settings::value("MainWindow/splitter/state").toByteArray());
    m_ui->actionExportToArduino->setEnabled(false);

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
    auto *shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(shortcut, &QShortcut::activated, m_ui->lineEditSearch, qOverload<>(&QWidget::setFocus));
    populateLeftMenu();
    m_ui->tabElements->setTabIcon(0, QIcon(":/input/buttonOff.png"));
    m_ui->tabElements->setTabIcon(1, QIcon(":/basic/xor.png"));
    m_ui->tabElements->setTabIcon(2, QIcon(DFlipFlop::pixmapPath()));
    m_ui->tabElements->setTabIcon(3, QIcon(":/basic/ic.png"));
    m_ui->tabElements->setTabIcon(4, QIcon(":/text.png"));
    m_ui->tabElements->setTabEnabled(5, false);

    qCDebug(zero) << "Loading recent file list.";
    m_recentFilesController = new RecentFilesController(this);
    connect(this, &MainWindow::addRecentFile, m_recentFilesController, &RecentFilesController::addRecentFile);
    createRecentFileActions();
    connect(m_recentFilesController, &RecentFilesController::recentFilesUpdated, this, &MainWindow::updateRecentFileActions);

    qCDebug(zero) << "Checking playing simulation.";
    m_ui->actionPlay->setChecked(true);

    qCDebug(zero) << "Connecting ICManager to MainWindow";
    connect(&ICManager::instance(), &ICManager::openIC, this, &MainWindow::loadPandaFile);

    qCDebug(zero) << "Setting connections";
    connect(m_ui->actionAbout,            &QAction::triggered,       this,                &MainWindow::on_actionAbout_triggered);
    connect(m_ui->actionAboutQt,          &QAction::triggered,       this,                &MainWindow::on_actionAboutQt_triggered);
    connect(m_ui->actionAboutThisVersion, &QAction::triggered,       this,                &MainWindow::aboutThisVersion);
    connect(m_ui->actionChangeTrigger,    &QAction::triggered,       m_ui->elementEditor, &ElementEditor::changeTriggerAction);
    connect(m_ui->actionDarkTheme,        &QAction::triggered,       this,                &MainWindow::on_actionDarkTheme_triggered);
    connect(m_ui->actionEnglish,          &QAction::triggered,       this,                &MainWindow::on_actionEnglish_triggered);
    connect(m_ui->actionExit,             &QAction::triggered,       this,                &MainWindow::on_actionExit_triggered);
    connect(m_ui->actionExportToImage,    &QAction::triggered,       this,                &MainWindow::on_actionExportToImage_triggered);
    connect(m_ui->actionExportToPdf,      &QAction::triggered,       this,                &MainWindow::on_actionExportToPdf_triggered);
    connect(m_ui->actionFastMode,         &QAction::triggered,       this,                &MainWindow::on_actionFastMode_triggered);
    connect(m_ui->actionFlipHorizontally, &QAction::triggered,       this,                &MainWindow::on_actionFlipHorizontally_triggered);
    connect(m_ui->actionFlipVertically,   &QAction::triggered,       this,                &MainWindow::on_actionFlipVertically_triggered);
    connect(m_ui->actionFullscreen,       &QAction::triggered,       this,                &MainWindow::on_actionFullscreen_triggered);
    connect(m_ui->actionGates,            &QAction::triggered,       this,                &MainWindow::on_actionGates_triggered);
    connect(m_ui->actionLabelsUnderIcons, &QAction::triggered,       this,                &MainWindow::on_actionLabelsUnderIcons_triggered);
    connect(m_ui->actionLightTheme,       &QAction::triggered,       this,                &MainWindow::on_actionLightTheme_triggered);
    connect(m_ui->actionMute,             &QAction::triggered,       this,                &MainWindow::on_actionMute_triggered);
    connect(m_ui->actionNew,              &QAction::triggered,       this,                &MainWindow::on_actionNew_triggered);
    connect(m_ui->actionOpen,             &QAction::triggered,       this,                &MainWindow::on_actionOpen_triggered);
    connect(m_ui->actionPlay,             &QAction::triggered,       this,                &MainWindow::on_actionPlay_triggered);
    connect(m_ui->actionPortuguese,       &QAction::triggered,       this,                &MainWindow::on_actionPortuguese_triggered);
    connect(m_ui->actionReloadFile,       &QAction::triggered,       this,                &MainWindow::on_actionReloadFile_triggered);
    connect(m_ui->actionRename,           &QAction::triggered,       m_ui->elementEditor, &ElementEditor::renameAction);
    connect(m_ui->actionResetZoom,        &QAction::triggered,       this,                &MainWindow::on_actionResetZoom_triggered);
    connect(m_ui->actionRotateLeft,       &QAction::triggered,       this,                &MainWindow::on_actionRotateLeft_triggered);
    connect(m_ui->actionRotateRight,      &QAction::triggered,       this,                &MainWindow::on_actionRotateRight_triggered);
    connect(m_ui->actionSave,             &QAction::triggered,       this,                &MainWindow::on_actionSave_triggered);
    connect(m_ui->actionSaveAs,           &QAction::triggered,       this,                &MainWindow::on_actionSaveAs_triggered);
    connect(m_ui->actionSelectAll,        &QAction::triggered,       this,                &MainWindow::on_actionSelectAll_triggered);
    connect(m_ui->actionWaveform,         &QAction::triggered,       this,                &MainWindow::on_actionWaveform_triggered);
    connect(m_ui->actionWires,            &QAction::triggered,       this,                &MainWindow::on_actionWires_triggered);
    connect(m_ui->actionZoomIn,           &QAction::triggered,       this,                &MainWindow::on_actionZoomIn_triggered);
    connect(m_ui->actionZoomOut,          &QAction::triggered,       this,                &MainWindow::on_actionZoomOut_triggered);
    connect(m_ui->lineEditSearch,         &QLineEdit::returnPressed, this,                &MainWindow::on_lineEditSearch_returnPressed);
    connect(m_ui->lineEditSearch,         &QLineEdit::textChanged,   this,                &MainWindow::on_lineEditSearch_textChanged);

    qCDebug(zero) << "Window title.";
    setWindowTitle("WiRedPanda " + QString(APP_VERSION));

    qCDebug(zero) << "Building a new tab.";
    createNewTab();

    qCDebug(zero) << "Opening file if not empty.";
    if (!fileName.isEmpty()) {
        loadPandaFile(fileName);
    }
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::loadAutosaveFiles()
{
    QStringList autosaves(Settings::value("autosaveFile").toStringList());

    qCDebug(zero) << "all autosave files:" << autosaves;

    for (auto i = autosaves.begin(); i != autosaves.end();) {
        QFile file(*i);

        if (!file.exists()) {
            qCDebug(zero) << "Removing autosave file name from config that does not exist.";
            i = autosaves.erase(i);
            continue;
        }

        loadPandaFile(*i);
        m_currentTab->setIsAutosave();

        ++i;
    }

    Settings::setValue("autosaveFile", autosaves);
}

void MainWindow::createNewTab()
{
    qCDebug(zero) << "Creating new workspace.";
    qCDebug(zero) << "Creating the tab structure.";
    qCDebug(zero) << "Adding tab. #tabs:" << m_ui->tab->count() << ", current tab:" << m_tabIndex;
    auto *workspace = new WorkSpace(this);
    connect(workspace, &WorkSpace::fileChanged, this, &MainWindow::setTabText);
    workspace->view()->setFastMode(m_ui->actionFastMode->isChecked());
    workspace->scene()->updateTheme();
    m_ui->tab->addTab(workspace, tr("New Project"));
    qCDebug(zero) << "Selecting the newly created tab.";
    m_ui->tab->setCurrentIndex(m_ui->tab->count() - 1);
    qCDebug(zero) << "Finished #tabs:" << m_ui->tab->count() << ", current tab:" << m_tabIndex;
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

bool MainWindow::save(const QString &fileName)
{
    if (m_currentTab) {
        m_currentTab->save(fileName);
    }

    return true;
}

void MainWindow::show()
{
    QMainWindow::show();
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
    // TODO: if current tab is empty dont ask user
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
    qCDebug(zero) << "Current file set.";
    setCurrentFile(fileName);
    qCDebug(zero) << "Loading in editor.";
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
    save();
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

QFileInfo MainWindow::currentFile() const
{
    return m_currentTab ? m_currentTab->fileInfo() : QFileInfo();
}

QDir MainWindow::currentDir() const
{
    return m_currentTab ? m_currentTab->fileInfo().absoluteDir() : QDir();
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    m_currentFile = fileInfo;
    m_ui->tab->setTabText(m_tabIndex, fileInfo.fileName());

    qCDebug(zero) << "Adding file to controller.";
    emit addRecentFile(fileInfo.absoluteFilePath());
}

void MainWindow::setTabText(const QString &tabText)
{
    m_ui->tab->setTabText(m_tabIndex, tabText);
}

void MainWindow::on_actionSelectAll_triggered()
{
    m_currentTab->scene()->selectAll();
}

void MainWindow::updateICList()
{
    m_ui->scrollAreaWidgetContents_IC->layout()->removeItem(m_ui->verticalSpacer_IC);

    auto items = m_ui->scrollAreaWidgetContents_IC->findChildren<ElementLabel *>();

    for (auto *item : qAsConst(items)) {
        item->deleteLater();
    }

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
            QPixmap pixmap(QString::fromUtf8(":/basic/ic.png"));
            auto *item = new ElementLabel(pixmap, ElementType::IC, file, this);
            m_ui->scrollAreaWidgetContents_IC->layout()->addWidget(item);
        }
    }

    m_ui->scrollAreaWidgetContents_IC->layout()->addItem(m_ui->verticalSpacer_IC);
}

bool MainWindow::closeTab(const int tabIndex)
{
    qCDebug(zero) << "Closing tab" << tabIndex + 1 << ", #tabs:" << m_ui->tab->count();

    auto *workspace = dynamic_cast<WorkSpace *>(m_ui->tab->widget(tabIndex));

    qCDebug(zero) << "Checking if needs to save file.";
    if (!workspace->scene()->undoStack()->isClean()) {
        const int selectedButton = confirmSave(false);

        if (selectedButton == QMessageBox::Cancel) {
            return false;
        }

        if (selectedButton == QMessageBox::Yes) {
            try {
                workspace->save();
            }  catch (std::exception &) {
                if (closeTabAnyway() == QMessageBox::No) {
                    return false;
                }
            }
        }
    }

    qCDebug(zero) << "Deleting tab.";
    workspace->deleteLater();
    m_ui->tab->removeTab(tabIndex);

    if (m_ui->tab->count() == 0) {
        m_currentTab = nullptr;
        m_tabIndex = -1;
    }

    qCDebug(zero) << "Closed tab" << tabIndex << ", #tabs:" << m_ui->tab->count() << ", current tab:" << m_tabIndex;

    return true;
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

    qCDebug(zero) << "Reinitialize simulation controller.";
    m_currentTab->simulationController()->start();
    m_currentTab->scene()->setHandlingEvents(true);
    m_currentTab->scene()->clearSelection();
    m_currentTab->view()->setFastMode(m_ui->actionFastMode->isChecked());
}

void MainWindow::selectTab(const int tabIndex)
{
    if (tabIndex == -1) {
        return;
    }

    if (m_currentTab) {
        disconnectTab(); // disconnect previously selected tab
    }

    m_currentTab = dynamic_cast<WorkSpace *>(m_ui->tab->currentWidget());
    m_tabIndex = m_ui->tab->currentIndex();
    qCDebug(zero) << "Selecting tab:" << tabIndex;
    connectTab();
    qCDebug(zero) << "New tab selected. BD fileName:" << m_currentTab->dolphinFileName();
}

void MainWindow::on_lineEditSearch_textChanged(const QString &text)
{
    m_ui->scrollAreaWidgetContents_Search->layout()->removeItem(m_ui->verticalSpacer_Search);

    if (text.isEmpty()) {
        m_ui->tabElements->tabBar()->show();
        m_ui->tabElements->setCurrentIndex(0);
        m_ui->tabElements->setTabEnabled(5, false);
    } else {
        m_ui->tabElements->tabBar()->hide();
        m_ui->tabElements->setCurrentIndex(5);
        m_ui->tabElements->setTabEnabled(5, true);

        auto ics = m_ui->scrollArea_Search->findChildren<ElementLabel *>("label_ic");
        QRegularExpression regex(QString(".*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        QRegularExpression regex2(QString("^label_.*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        auto searchResults = m_ui->scrollArea_Search->findChildren<ElementLabel *>(regex2);
        auto allItems = m_ui->scrollArea_Search->findChildren<ElementLabel *>();

        for (auto *item : qAsConst(allItems)) {
            if (regex.match(item->name()).hasMatch()) {
                if (!searchResults.contains(item)) {
                    searchResults.append(item);
                }
            }
        }

        for (auto *ic : qAsConst(ics)) {
            if (regex.match(ic->icFileName()).hasMatch()) {
                searchResults.append(ic);
            }
        }

        for (auto *item : qAsConst(allItems)) {
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
    auto allLabels = m_ui->scrollArea_Search->findChildren<ElementLabel *>();

    for (auto *label : allLabels) {
        if (label->isVisible()) {
            m_currentTab->scene()->addItem(label->mimeData());
            m_ui->lineEditSearch->clear();
            return;
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

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
            if (!save()) {
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
        m_ui->menuRecentFiles->setEnabled(true);
    }
    auto actions = m_ui->menuRecentFiles->actions();
    for (int i = 0; i < numRecentFiles; ++i) {
        const QString text = "&" + QString::number(i + 1) + " " + QFileInfo(files.at(i)).fileName();
        actions.at(i)->setText(text);
        actions.at(i)->setData(files[i]);
        actions.at(i)->setVisible(true);
    }
    for (int i = numRecentFiles; i < RecentFilesController::MaxRecentFiles; ++i) {
        actions.at(i)->setVisible(false);
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
    m_ui->menuRecentFiles->clear();

    for (int i = 0; i < RecentFilesController::MaxRecentFiles; ++i) {
        auto *action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
        m_ui->menuRecentFiles->addAction(action);
    }
    updateRecentFileActions();
}

void MainWindow::on_actionExportToPdf_triggered()
{
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
}

void MainWindow::on_actionExportToImage_triggered()
{
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
}

void MainWindow::retranslateUi()
{
    m_ui->retranslateUi(this);
    m_ui->elementEditor->retranslateUi();

    auto items = m_ui->tabElements->findChildren<ElementLabel *>();
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

    auto *simController = m_currentTab->simulationController();

    checked ? simController->start() : simController->stop();
    simController->updateAll();
}

// TODO: call this function with the list of files and set directory watcher.
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
    populateMenu(m_ui->verticalSpacer_InOut, {"InputVcc", "InputGnd", "InputButton", "InputSwitch", "InputRotary", "Clock", "Led", "Display", "Display14", "Buzzer"}, m_ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(m_ui->verticalSpacer_Gates, {"And", "Or", "Not", "Nand", "Nor", "Xor", "Xnor", "Mux", "Demux", "Node"}, m_ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(m_ui->verticalSpacer_Memory, {"DLatch", "DFlipFlop", "JKFlipFlop", "SRFlipFlop", "TFlipFlop"}, m_ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(m_ui->verticalSpacer_Misc, {"Text", "Line"}, m_ui->scrollAreaWidgetContents_Misc->layout());
}

void MainWindow::on_actionFastMode_triggered(const bool checked)
{
    setFastMode(checked);
    Settings::setValue("fastMode", checked);
}

void MainWindow::on_actionWaveform_triggered()
{
    if (!m_currentFile.exists()) {
        throw Pandaception("Save file before opening waveform!");
    }

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

    m_ui->tabElements->setTabIcon(2, QIcon(DFlipFlop::pixmapPath()));

    auto labels = m_ui->memory->findChildren<ElementLabel *>();

    for (auto label : labels) {
        label->updateTheme();
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
    switch (event->type()) {
    case QEvent::WindowActivate: {
        if (m_ui->actionPlay->isChecked()) {
            on_actionPlay_triggered(true);
        }
        break;
    }

    case QEvent::WindowDeactivate: {
        on_actionPlay_triggered(false);
        break;
    }

    default: break;
    };

    return QMainWindow::event(event);
}
