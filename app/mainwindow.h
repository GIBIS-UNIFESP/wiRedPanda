#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "bewaveddolphin.h"
#include "editor.h"
#include "graphicsview.h"
#include "graphicsviewzoom.h"
#include "label.h"
#include "listitemwidget.h"
#include "recentfilescontroller.h"
#include "scene.h"

#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QSpacerItem>
#include <QTemporaryFile>
#include <QTranslator>
#include <QUndoView>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, QString filename = QString());
    ~MainWindow();

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
    void setCurrentFile(const QFileInfo &value);

    //! Exports the current simulation to an
    bool exportToArduino(QString fname);
    //! Saves the current Bewaved Dolphin (waveform simulator) file
    bool exportToWaveFormFile(QString fname);

    //! Loads a .panda file
    bool loadPandaFile(const QString &fname);
    void createUndoView();
    //! Opens a message box asking the user if he wishes to save his progress
    int confirmSave();
    //!
    void updateRecentICs();

    QString getOpenICFile();

    bool closeFile();

    void populateMenu(QSpacerItem *spacer, QString names, QLayout *layout);

    void retranslateUi();

    void loadTranslation(QString language);

    void setFastMode(bool fastModeEnabled);

    void buildFullScreenDialog();

    QString getDolphinFilename();
    void setDolphinFilename(QString filename);

    QDialog *fullscreenDlg;
    GraphicsView *fullscreenView;

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
    int recoverAutoSaveFile(QString autosaveFilename);
    /* QWidget interface */
protected:
    void closeEvent(QCloseEvent *e);
    void resizeEvent(QResizeEvent *);
};

#endif /* MAINWINDOW_H */
