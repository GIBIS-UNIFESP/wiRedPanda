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
class QUndoView;
class QSpacerItem;
class QTranslator;

class BewavedDolphin;
class Editor;
class GraphicsView;
class Label;
class ListItemWidget;

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

    //! Saves the project to a .panda file. Removes the autosave file in the process.
    bool save(QString fname = QString());

    //! Sets the main window as visible, as well as its child widgets. Cleans the editor.
    void show();

    //!
    void clear();

    //! Returns the file name of the currently loaded Panda file.
    QFileInfo getCurrentFile() const;

    //! Sets the current file to the given value.
    //! Mostly used by `loadPandaFile` and clearing functions
    void setCurrentFile(const QFileInfo &file);

    //! Exports the current simulation to an
    bool exportToArduino(QString fname);
    //! Saves the current Bewaved Dolphin (waveform simulator) file
    bool exportToWaveFormFile(const QString& fname);

    //! Loads a .panda file
    bool loadPandaFile(const QString &fname);
    void createUndoView();
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

    QDialog *fullscreenDlg;
    GraphicsView *fullscreenView;

signals:
    void addRecentIcFile(const QString &fname);
    void addRecentFile(const QString &fname);

public slots:
    void updateRecentICs();

private slots:
    
    /* Menu File  Start*/
    void actionNewTriggered();
    void actionOpenTriggered();
    
    void actionOpenIcTriggered();
    void openRecentFile();
    void actionReloadFileTriggered();
    
    void actionSaveTriggered();
    void actionSaveAsTriggered();
    void actionSaveLocalProjectTriggered();
    
    bool actionExportToArduinoTriggered();
    void actionPrintTriggered();
    void actionExportToImageTriggered();
    void actionExitTriggered();
        
    
    /* Menu Edit */
    void actionRenameTriggered();
    void actionChangeTriggerTriggered();
    
    void actionRotateLeftTriggered();
    void actionRotateRightTriggered();
    void actionFlipHorizontallyTriggered();
    void actionFlipVerticallyTriggered();
    
    void actionSelectAllTriggered();
    void actionClearSelectionTriggered();
    
    
    /* Menu View */
    void actionPandaLightTriggered();
    void actionPandaDarkTriggered();
    void actionZoomInTriggered();
    void actionZoomOutTriggered();
    void actionResetZoomTriggered();
    void actionWiresTriggered(bool checked);
    void actionGatesTriggered(bool checked);
    void actionFastModeTriggered(bool checked);
    void actionFullScreenTriggered() const;
    void actionLabelsUnderIconsTriggered(bool checked);
    
    
    
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_triggered();
    void on_actionEnglish_triggered();
    
   
    
    
    
    void on_actionPortuguese_triggered();
    
   
   
    
    
    void on_lineEdit_returnPressed();
    void on_lineEdit_textChanged(const QString &arg1);
    
    void scrollView(int dx, int dy);
    void updateRecentFileActions();
    void zoomChanged();

    void on_actionPlay_triggered(bool checked);

   

    

    void on_actionWaveform_triggered();

    

    

    void updateTheme();

    

  

    void autoSave();

    void on_actionMute_triggered();

    

    

private:
    Ui::MainWindow *ui;
    Editor *editor;
    QFileInfo currentFile;
    QDir defaultDirectory;
    QUndoView *undoView;
    Label *firstResult;
    bool loadedAutosave;
    QString autosaveFilename;
    QString dolphinFilename;
    BewavedDolphin *bd;

    QTemporaryFile autosaveFile;

    QAction *undoAction;
    QAction *redoAction;
    RecentFilesController *rfController, *ricController;
    QAction *recentFileActs[RecentFilesController::MaxRecentFiles];
    QTranslator *translator;
    QVector<ListItemWidget *> boxItemWidgets, searchItemWidgets;
    void createRecentFileActions();
    void populateLeftMenu();
    // Shows a message box for reloading the autosave at launch, when
    // there's reason to believe that there's been unsaved progress.
    int recoverAutoSaveFile(const QString& autosaveFilename);
    /* QWidget interface */
protected:
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *) override;
};

#endif /* MAINWINDOW_H */
