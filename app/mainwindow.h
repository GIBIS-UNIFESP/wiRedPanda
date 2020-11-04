#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow( QWidget *parent = nullptr );
  ~MainWindow( );

  bool save( QString fname = QString( ) );
  void show( );
  void clear( );
  QFileInfo getCurrentFile( ) const;
  void setCurrentFile( const QFileInfo &value );
  bool ExportToArduino( QString fname );
  bool ExportToWaveFormFile( QString fname );

  bool open( const QString &fname );
  void createUndoView( );
  int confirmSave( );
  void updateRecentBoxes( );

  QString getOpenBoxFile( );

  bool closeFile( );


  void populateMenu( QSpacerItem *spacer, QString names, QLayout *layout );

  void retranslateUi( );

  void loadTranslation( QString language );

  void setFastMode( bool fastModeEnabled );

  void buildFullScreenDialog( );

  QDialog *fullscreenDlg;
  GraphicsView *fullscreenView;

private slots:
  bool on_actionExport_to_Arduino_triggered( );
  void on_actionExport_to_Image_triggered( );
  void on_actionPrint_triggered( );

  void on_actionAbout_Qt_triggered( );
  void on_actionAbout_triggered( );
  void on_actionEnglish_triggered( );
  void on_actionExit_triggered( );
  void on_actionGates_triggered( bool checked );
  void on_actionNew_triggered( );
  void on_actionOpen_Box_triggered( );
  void on_actionOpen_triggered( );
  void on_actionPortuguese_triggered( );
  void on_actionReload_File_triggered( );
  void on_actionReset_Zoom_triggered( );
  void on_actionRotate_left_triggered( );
  void on_actionRotate_right_triggered( );
  void on_actionSave_As_triggered( );
  void on_actionSave_triggered( );
  void on_actionSelect_all_triggered( );
  void on_actionWires_triggered( bool checked );
  void on_actionZoom_in_triggered( );
  void on_actionZoom_out_triggered( );
  void on_lineEdit_returnPressed( );
  void on_lineEdit_textChanged( const QString &arg1 );
  void openRecentFile( );
  void scrollView( int dx, int dy );
  void updateRecentFileActions( );
  void zoomChanged( );

  void on_actionPlay_triggered( bool checked );

  void on_actionRename_triggered( );

  void on_actionChange_Trigger_triggered( );

  void on_actionClear_selection_triggered( );


  void on_actionFast_Mode_triggered( bool checked );

  void on_actionWaveform_triggered( );

  void on_actionPanda_Light_triggered( );

  void on_actionPanda_Dark_triggered( );

  void updateTheme( );

  void on_actionFlip_horizontally_triggered( );

  void on_actionFlip_vertically_triggered( );

  void on_actionFullscreen_triggered( );

  void autoSave( );

  void on_actionMute_triggered( );

  void on_actionLabels_under_icons_triggered(bool checked);

private:
  Ui::MainWindow *ui;
  Editor *editor;
  QFileInfo currentFile;
  QDir defaultDirectory;
  QUndoView *undoView;
  Label *firstResult;

  QTemporaryFile autosaveFile;

  QAction *undoAction;
  QAction *redoAction;
  RecentFilesController *rfController, *rboxController;
  QAction *recentFileActs[ RecentFilesController::MaxRecentFiles ];
  QTranslator *translator;
  QVector< ListItemWidget* > boxItemWidgets, searchItemWidgets;
  void createRecentFileActions( );
  void populateLeftMenu( );
  /* QWidget interface */
protected:
  void closeEvent( QCloseEvent *e );
  void resizeEvent( QResizeEvent* );
};

#endif /* MAINWINDOW_H */
