/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QFileInfo>
#include <QMainWindow>
#include <QTemporaryFile>

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
    void createNewTab(const QString &tab_name);
    //! Saves the project to a .panda file. Removes the autosave file in the process.
    bool save(QString fname = QString());
    //! Sets the main window as visible, as well as its child widgets. Cleans the editor.
    void show();
    //! Returns the file name of the currently loaded Panda file.
    QFileInfo getCurrentFile() const;
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
    int confirmSave();
    //!
    QString getOpenICFile();
    bool closeFile();
    void populateMenu(QSpacerItem *spacer, const QString& names, QLayout *layout);
    void retranslateUi();
    void loadTranslation(const QString& language);
    void setFastMode(bool fastModeEnabled);
    void buildFullScreenDialog();
    QString getDolphinFilename();
    void setDolphinFilename(const QString &filename);

    QDialog *m_fullscreenDlg;
    GraphicsView *m_fullscreenView;

signals:
    void addRecentIcFile(const QString &fname);
    void addRecentFile(const QString &fname);

public slots:
    void updateRecentICs();
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
    void on_actionOpen_IC_triggered();
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
    void on_actionSave_Local_Project_triggered();

private:
    Ui::MainWindow *ui;
    Editor *m_editor;
    QFileInfo m_currentFile;
    QDir m_defaultDirectory;
    Label *m_firstResult;
    bool m_loadedAutoSave;
    QString m_autoSaveFileName;
    QString m_dolphinFileName;
    BewavedDolphin *m_bd;
    QVector<WorkSpace> m_tabs;
    int m_current_tab;
    QVector<QTemporaryFile*> m_autoSaveFile;

    QVector<QAction*> m_undoAction;
    QVector<QAction*> m_redoAction;
    RecentFilesController *m_rfController, *m_ricController;
    QAction *m_recentFileActs[RecentFilesController::MaxRecentFiles];
    QTranslator *m_translator;
    QVector<ListItemWidget *> icItemWidgets, searchItemWidgets;
    /**
     * @brief createNewWorkspace: Creates a new workspace in a new tab. Called by load and new functions.
     * @param fname
     */
    void createNewWorkspace(const QString &fname);
    void createRecentFileActions();
    void populateLeftMenu();
    // Shows a message box for reloading the autosave at launch, when
    // there's reason to believe that there's been unsaved progress.
    int recoverAutoSaveFile(const QString& autosaveFilename);
    // Undo and Redo interface for each tab.
    void createUndoRedoMenus();
    // Change the undo and redo menu when another tab is selected.
    void selectUndoRedoMenu(int tab);
protected:
    /* QWidget interface */
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *) override;
};

#endif /* MAINWINDOW_H */
