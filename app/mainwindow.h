/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QFileInfo>
#include <QMainWindow>
#include <QSettings>
#include <QTemporaryFile>
#include <QDomDocument>

#include "recentfilescontroller.h"

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
    explicit MainWindow(QWidget *parent = nullptr, const QString &filename = QString());
    ~MainWindow() override;
    //! Creates a new tab with the given tab_name. Used by new and load actions.
    void createNewTab();
    //! Saves the project to a .panda file. Removes the autosave file in the process.
    bool save(QString fname = QString());
    //! Sets the main window as visible, as well as its child widgets. Cleans the editor.
    void show();
    //! Returns the file name of the currently loaded Panda file.
    QFileInfo getCurrentFile() const;
    //! Returns the dir name of the currently loaded Panda file.
    QDir getCurrentDir() const;
    //! Sets the current file to the given value.
    //! Mostly used by `loadPandaFile` and clearing functions
    void setCurrentFile(const QFileInfo &file);
    void setAutoSaveFileName(const QFileInfo &file);
    //! Exports the current simulation to an
    bool exportToArduino(QString fname);
    //! Saves the current Bewaved Dolphin (waveform simulator) file
    bool exportToWaveFormFile(const QString& fname);
    //! Loads a .panda file
    bool loadPandaFile(const QString &fname);
    //! Opens a message box asking the user if he wishes to save his progress
    int confirmSave(bool multiple=true);
    bool closeFile();
    void populateMenu(QSpacerItem *spacer, const QString& names, QLayout *layout);
    void retranslateUi();
    void loadTranslation(const QString& language);
    void setFastMode(bool fastModeEnabled);
    void buildFullScreenDialog();
    QString getDolphinFilename();
    void setDolphinFilename(const QString &filename);

    QDomDocument* loadRemoteFunctions( );

    QDialog *m_fullscreenDlg;
    GraphicsView *m_fullscreenView;

    void setCurrentDir();
signals:
    void addRecentFile(const QString &fname);

public slots:
    void updateICList();
    void closeTab(int tab);
    void selectTab(int tab);

private slots:
    bool on_actionExport_to_Arduino_triggered();
    void on_actionExport_to_Image_triggered();
    void on_actionPrint_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_triggered();
    void on_actionEnglish_triggered();
    void on_actionExit_triggered();
    void on_actionGates_triggered(bool checked);
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionPortuguese_triggered();
    void on_actionReload_File_triggered();
    void on_actionReset_Zoom_triggered();
    void on_actionRotate_left_triggered();
    void on_actionRotate_right_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_triggered();
    void on_actionSelect_all_triggered();
    void on_actionWires_triggered(bool checked);
    void on_actionZoom_in_triggered();
    void on_actionZoom_out_triggered();
    void on_lineEdit_returnPressed();
    void on_lineEdit_textChanged(const QString &arg1);
    void openRecentFile();
    void scrollView(int dx, int dy);
    void updateRecentFileActions();
    void zoomChanged();
    void on_actionPlay_triggered(bool checked);
    void on_actionRename_triggered();
    void on_actionChange_Trigger_triggered();
    void on_actionClear_selection_triggered();
    void on_actionFast_Mode_triggered(bool checked);
    void on_actionWaveform_triggered();
    void on_actionPanda_Light_triggered();
    void on_actionPanda_Dark_triggered();
    void updateTheme();
    void on_actionFlip_horizontally_triggered();
    void on_actionFlip_vertically_triggered();
    void on_actionFullscreen_triggered() const;
    void autoSave();
    void on_actionMute_triggered();
    void on_actionLabels_under_icons_triggered(bool checked);
    void aboutThisVersion();
private:
    Ui::MainWindow *ui;
    Editor *m_editor;
    QFileInfo m_currentFile;
    QDir m_defaultDirectory;
    Label *m_firstResult;
    bool m_loadedAutoSave;
    QString m_dolphinFileName;
    BewavedDolphin *m_bd;
    QVector<WorkSpace> m_tabs;
    int m_current_tab;
    QTemporaryFile** m_autoSaveFile; // We had memory leak problems using QVectors for that.

    QVector<QAction*> m_undoAction;
    QVector<QAction*> m_redoAction;
    RecentFilesController *m_rfController;
    QAction *m_recentFileActs[RecentFilesController::MaxRecentFiles];
    QTranslator *m_translator;
    QVector<ListItemWidget *> icItemWidgets, searchItemWidgets;
    /**
     * @brief createNewWorkspace: Creates a new workspace in a new tab. Called by load and new functions.
     */
    void createNewWorkspace();
    void createRecentFileActions();
    void populateLeftMenu();
    // Shows a message box for reloading the autosave at launch, when
    // there's reason to believe that there's been unsaved progress.
    int recoverAutoSaveFile(const QString& autosaveFilename);
    int autoSaveFileDeleteAnyway(const QString& autosaveFilename);
    // Undo and Redo interface for each tab.
    void createUndoRedoMenus();
    /**
     * @brief addUndoRedoMenu: Adds undo and redo of selected tab into the UI menu.
     */
    void addUndoRedoMenu(int tab);
    /**
     * @brief removeUndoRedoMenu: Removes undo and redo of current tab from the UI menu.
     */
    void removeUndoRedoMenu();
    void loadAutoSaveFiles(QSettings &settings, const QString &filename);
    bool closeTabAction(int tab);
    // Message box to ask if user wants to close in case of canceled or failed save action.
    int closeTabAnyway();
    // Checks if any tab has content to be saved.
    bool hasModifiedFiles();
    void updateSettings();
    void createAutosaveFile();
    void removeAutosaveFile(int tab);
    /**
     * @brief disconnectTab: Function used to disconnect elements of current tab, to safely change or close a tab.
     */
    void disconnectTab();
    /**
     * @brief connectTab: Function called as a tab is selected. The tab is connected to the UI.
     */
    void connectTab(int tab);
    void selectNextTab();
    void selectPreviousTab();
protected:
    /* QWidget interface */
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *) override;
};

#endif /* MAINWINDOW_H */
