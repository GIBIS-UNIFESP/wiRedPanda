/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "recentfilescontroller.h"

#include <QDir>
#include <QFileInfo>
#include <QMainWindow>
#include <QTemporaryFile>

class QDialog;
class QSpacerItem;
class QTranslator;

class BewavedDolphin;
class Editor;
class GraphicsView;
class Label;
class ListItemWidget;
class WorkSpace;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, const QString &filename = {});
    ~MainWindow() override;

    //! Creates a new tab with the given tab_name. Used by new and load actions.
    void createNewTab();
    //! Saves the project to a .panda file. Removes the autosave file in the process.
    bool save(QString fname = {});
    //! Sets the main window as visible, as well as its child widgets. Cleans the editor.
    void show();
    //! Returns the file name of the currently loaded Panda file.
    QFileInfo getCurrentFile() const;
    //! Returns the dir name of the currently loaded Panda file.
    QDir getCurrentDir() const;
    //! Sets the current file to the given value.
    //! Mostly used by `loadPandaFile` and clearing functions
    void setCurrentFile(const QString &fname);
    //! Exports the current simulation to an
    bool exportToArduino(QString fname);
    //! Saves the current beWavedDolphin (waveform simulator) file
    bool exportToWaveFormFile(const QString &fname);
    //! Loads a .panda file
    bool loadPandaFile(const QString &fname);
    //! Opens a message box asking the user if he wishes to save his progress
    int confirmSave(bool multiple = true);

    QString getDolphinFilename();
    bool closeFile();
    void buildFullScreenDialog();
    void loadTranslation(QString language);
    void populateMenu(QSpacerItem *spacer, const QString &names, QLayout *layout);
    void retranslateUi();
    void setAutoSaveFileName(const QFileInfo &file);
    void setCurrentDir();
    void setDolphinFilename(const QString &filename);
    void setFastMode(bool fastModeEnabled);

    QDialog *m_fullscreenDlg;
    GraphicsView *m_fullscreenView;


signals:
    void addRecentFile(const QString &fname);

protected:
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent * /*event*/) override;

private:
    bool closeTabAction(int tab);
    bool hasModifiedFiles();
    bool on_actionExport_to_Arduino_triggered();
    int autoSaveFileDeleteAnyway(const QString &autosaveFilename);
    int closeTabAnyway();
    int recoverAutoSaveFile(const QString &autosaveFilename);
    void aboutThisVersion();
    void autoSave();
    void closeTab(int tab);
    void createAutosaveFile();
    void createRecentFileActions();
    void createUndoRedoMenus();
    void loadAutoSaveFiles(const QString &filename);
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_triggered();
    void on_actionChange_Trigger_triggered();
    void on_actionClear_selection_triggered();
    void on_actionEnglish_triggered();
    void on_actionExit_triggered();
    void on_actionExport_to_Image_triggered();
    void on_actionFast_Mode_triggered(bool checked);
    void on_actionFlip_horizontally_triggered();
    void on_actionFlip_vertically_triggered();
    void on_actionFullscreen_triggered();
    void on_actionGates_triggered(bool checked);
    void on_actionLabels_under_icons_triggered(bool checked);
    void on_actionMute_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionPanda_Dark_triggered();
    void on_actionPanda_Light_triggered();
    void on_actionPlay_triggered(bool checked);
    void on_actionPortuguese_triggered();
    void on_actionPrint_triggered();
    void on_actionReload_File_triggered();
    void on_actionRename_triggered();
    void on_actionReset_Zoom_triggered() const;
    void on_actionRotate_left_triggered();
    void on_actionRotate_right_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_triggered();
    void on_actionSelect_all_triggered();
    void on_actionWaveform_triggered();
    void on_actionWires_triggered(bool checked);
    void on_actionZoom_in_triggered() const;
    void on_actionZoom_out_triggered() const;
    void on_lineEdit_returnPressed();
    void on_lineEdit_textChanged(const QString &text);
    void openRecentFile();
    void populateLeftMenu();
    void removeAutosaveFile(int tab);
    void scrollView(int dx, int dy) const;
    void selectNextTab();
    void selectPreviousTab();
    void selectTab(int tab);
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
    void addUndoRedoMenu(int tab);
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
    void connectTab(int tab);

    Ui::MainWindow *ui;
    BewavedDolphin *m_bd;
    Editor *m_editor;
    Label *m_firstResult;
    QAction *m_recentFileActs[RecentFilesController::MaxRecentFiles];
    QFileInfo m_currentFile;
    QString m_defaultDirectory;
    QString m_dolphinFileName;
    QTemporaryFile **m_autoSaveFile; // We had memory leak problems using QVectors for that.
    QTranslator *m_translator;
    QVector<ListItemWidget *> icItemWidgets, searchItemWidgets;
    QVector<QAction *> m_redoAction;
    QVector<QAction *> m_undoAction;
    QVector<WorkSpace> m_tabs;
    RecentFilesController *m_rfController;
    bool m_loadedAutoSave;
    int m_current_tab;
};

