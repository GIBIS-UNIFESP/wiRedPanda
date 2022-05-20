/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QDir>
#include <QMainWindow>
#include <QSpacerItem>
#include <QTranslator>

class Label;
class ListItemWidget;
class RecentFilesController;
class WorkSpace;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &fileName = {}, QWidget *parent = nullptr);
    ~MainWindow() override;

    //! Creates a new tab with the given tab_name. Used by new and load actions.
    void createNewTab();
    //! Saves the project to a .panda file. Removes the autosave file in the process.
    bool save(QString fileName = {});
    //! Sets the main window as visible, as well as its child widgets. Cleans the editor.
    void show();
    //! Returns the file name of the currently loaded Panda file.
    QFileInfo currentFile() const;
    //! Returns the dir name of the currently loaded Panda file.
    QDir currentDir() const;
    //! Sets the current file to the given value.
    //! Mostly used by `loadPandaFile` and clearing functions
    void setCurrentFile(const QString &fileName = {});
    //! Exports the current simulation to an
    void exportToArduino(QString fileName);
    //! Saves the current beWavedDolphin (waveform simulator) file
    void exportToWaveFormFile(const QString &fileName);
    //! Loads a .panda file
    bool loadPandaFile(const QString &fileName);
    //! Opens a message box asking the user if he wishes to save his progress
    int confirmSave(const bool multiple = true);

    QString dolphinFileName();
    bool closeFiles();
    void exportToWaveFormTerminal();
    void loadTranslation(const QString &language);
    void populateMenu(QSpacerItem *spacer, const QStringList &names, QLayout *layout);
    void retranslateUi();
    void setAutosaveFileName(const QFileInfo &fileInfo);
    void setCurrentDir();
    void setDolphinFileName(const QString &fileName);
    void setFastMode(const bool fastMode);

signals:
    void addRecentFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    bool closeTab(const int tabIndex);
    bool hasModifiedFiles();
    int autosaveFileDeleteAnyway(const QString &autosaveFileName);
    int closeTabAnyway();
    int recoverAutosaveFile(const QString &autosaveFileName);
    void aboutThisVersion();
    void autosave();
    void createAutosaveFile();
    void createRecentFileActions();
    void loadAutosaveFiles(const QString &fileName);
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionDarkTheme_triggered();
    void on_actionEnglish_triggered();
    void on_actionExit_triggered();
    void on_actionExportToArduino_triggered();
    void on_actionExportToImage_triggered();
    void on_actionExportToPdf_triggered();
    void on_actionFastMode_triggered(const bool checked);
    void on_actionFlipHorizontally_triggered();
    void on_actionFlipVertically_triggered();
    void on_actionFullscreen_triggered();
    void on_actionGates_triggered(const bool checked);
    void on_actionLabelsUnderIcons_triggered(const bool checked);
    void on_actionLightTheme_triggered();
    void on_actionMute_triggered(const bool checked);
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionPlay_triggered(const bool checked);
    void on_actionPortuguese_triggered();
    void on_actionReloadFile_triggered();
    void on_actionResetZoom_triggered() const;
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSave_triggered();
    void on_actionSelectAll_triggered();
    void on_actionWaveform_triggered();
    void on_actionWires_triggered(const bool checked);
    void on_actionZoomIn_triggered() const;
    void on_actionZoomOut_triggered() const;
    void on_lineEdit_returnPressed();
    void on_lineEdit_textChanged(const QString &text);
    void openRecentFile();
    void populateLeftMenu();
    void removeAutosaveFile(const int tabIndex);
    void selectTab(const int tabIndex);
    void updateICList();
    void updateRecentFileActions();
    void updateSettings();
    void updateTheme();
    void zoomChanged();

    /**
     * @brief createNewWorkspace: Creates a new workspace in a new tab. Called by load and new functions.
     */
    void createNewWorkspace();
    /**
     * @brief addUndoRedoMenu: Adds undo and redo of selected tab into the UI menu.
     */
    void addUndoRedoMenu();
    /**
     * @brief removeUndoRedoMenu: Removes undo and redo of current tab from the UI menu.
     */
    void removeUndoRedoMenu();
    /**
     * @brief disconnectTab: Function used to disconnect elements of current tab, to safely change or close a tab.
     */
    void disconnectTab();
    /**
     * @brief connectTab: Function called as a tab is selected. The tab is connected to the UI.
     */
    void connectTab();

    Ui::MainWindow *m_ui;
    Label *m_firstResult = nullptr;
    QFileInfo m_currentFile;
    QString m_defaultDirectory;
    QTranslator *m_pandaTranslator = nullptr;
    QTranslator *m_qtTranslator = nullptr;
    QVector<ListItemWidget *> icItemWidgets;
    QVector<ListItemWidget *> searchItemWidgets;
    QVector<QAction *> m_recentFilesActions;
    QVector<QTemporaryFile *> m_autosaveFile;
    RecentFilesController *m_recentFilesController = nullptr;
    WorkSpace *m_currentTab = nullptr;
    bool m_loadedAutosave = false;
    int m_tabIndex = -1;
};

