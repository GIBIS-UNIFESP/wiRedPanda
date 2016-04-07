#include "globalproperties.h"
#include "graphicsviewzoom.h"
#include "listitemwidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QRectF>
#include <QShortcut>
#include <QStyleFactory>
#include <QtPrintSupport/QPrinter>
#include <arduino/codegenerator.h>
#include <cmath>
#include <iostream>
#include <stdexcept>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
  ui->setupUi( this );
  editor = new Editor( this );
  ui->graphicsView->setScene( editor->getScene( ) );
/*  ui->graphicsView->setBackgroundBrush(QBrush(QColor(Qt::gray))); */
  ui->graphicsView->setRenderHint( QPainter::Antialiasing, true );
  ui->graphicsView->setRenderHint( QPainter::HighQualityAntialiasing, true );
  ui->graphicsView->setRenderHint( QPainter::SmoothPixmapTransform, true );

  ui->graphicsView->setAcceptDrops( true );
  editor->setElementEditor( ui->widgetElementEditor );
  ui->searchScrollArea->hide( );
  setCurrentFile( QFileInfo( ) );
#ifdef DEBUG
  createUndoView( );
#endif
  connect( ui->actionCopy, &QAction::triggered, editor, &Editor::copyAction );
  connect( ui->actionCut, &QAction::triggered, editor, &Editor::cutAction );
  connect( ui->actionPaste, &QAction::triggered, editor, &Editor::pasteAction );
  connect( ui->actionDelete, &QAction::triggered, editor, &Editor::deleteAction );

  undoAction = editor->getUndoStack( )->createUndoAction( this, tr( "&Undo" ) );
  undoAction->setIcon( QIcon( QPixmap( ":/toolbar/undo.png" ) ) );
  undoAction->setShortcuts( QKeySequence::Undo );

  redoAction = editor->getUndoStack( )->createRedoAction( this, tr( "&Redo" ) );
  redoAction->setIcon( QIcon( QPixmap( ":/toolbar/redo.png" ) ) );
  redoAction->setShortcuts( QKeySequence::Redo );

  ui->menuEdit->insertAction( ui->menuEdit->actions( ).at( 0 ), undoAction );
  ui->menuEdit->insertAction( undoAction, redoAction );
  gvzoom = new GraphicsViewZoom( ui->graphicsView );
  connect( gvzoom, &GraphicsViewZoom::zoomed, this, &MainWindow::zoomChanged );

  connect( editor, &Editor::scroll, this, &MainWindow::scrollView );

  rfController = new RecentFilesController( "recentFileList", this );
  rboxController = new RecentFilesController( "recentBoxes", this );

  QShortcut *shortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ), this );
  connect( shortcut, SIGNAL( activated( ) ), ui->lineEdit, SLOT( setFocus( ) ) );
  ui->graphicsView->setCacheMode( QGraphicsView::CacheBackground );
  firstResult = nullptr;
  updateRecentBoxes( );
  createRecentFileActions( );
  updateRecentFileActions( );

  connect( rfController, &RecentFilesController::recentFilesUpdated, this, &MainWindow::updateRecentFileActions );

  QApplication::setStyle( QStyleFactory::create( "Fusion" ) );
}

void MainWindow::createUndoView( ) {
  undoView = new QUndoView( editor->getUndoStack( ) );
  undoView->setWindowTitle( tr( "Command List" ) );
  undoView->show( );
  undoView->setAttribute( Qt::WA_QuitOnClose, false );
}


MainWindow::~MainWindow( ) {
  delete ui;
}

void MainWindow::on_actionExit_triggered( ) {
  close( );
}

bool MainWindow::save( QString fname = QString( ) ) {
  if( fname.isEmpty( ) ) {
    fname = currentFile.absoluteFilePath( );
    if( currentFile.fileName( ).isEmpty( ) ) {
      fname =
        QFileDialog::getSaveFileName( this, tr( "Save File" ), defaultDirectory.absolutePath( ), tr(
                                        "Panda files (*.panda)" ) );
    }
  }
  if( fname.isEmpty( ) ) {
    return( false );
  }
  if( !fname.endsWith( ".panda" ) ) {
    fname.append( ".panda" );
  }
  QFile fl( fname );
  if( fl.open( QFile::WriteOnly ) ) {
    QDataStream ds( &fl );
    try {
      editor->save( ds );
    }
    catch( std::runtime_error &e ) {
      std::cerr << tr( "Error saving project: " ).toStdString( ) << e.what( ) << std::endl;
      return( false );
    }
  }
  else {
    std::cerr << tr( "Could not open file in WriteOnly mode : " ).toStdString( ) << fname.toStdString( ) << "." <<
      std::endl;
    return( false );
  }
  fl.flush( );
  fl.close( );
  setCurrentFile( QFileInfo( fname ) );
  ui->statusBar->showMessage( tr( "Saved file sucessfully." ), 2000 );
  editor->getUndoStack( )->setClean( );
  return( true );
}

void MainWindow::show( ) {
  QMainWindow::show( );
  editor->clear( );
}

void MainWindow::clear( ) {
  editor->clear( );
  setCurrentFile( QFileInfo( ) );
}

int MainWindow::confirmSave( ) {
  QMessageBox msgBox;
  msgBox.setParent( this );
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
  msgBox.setText( tr( "Do you want to save your changes?" ) );
  msgBox.setWindowModality( Qt::WindowModal );
  msgBox.setDefaultButton( QMessageBox::Save );

  return( msgBox.exec( ) );
}

void MainWindow::on_actionNew_triggered( ) {
  if( closeFile( ) ) {
    clear( );
  }
}

void MainWindow::on_actionWires_triggered( bool checked ) {
  editor->showWires( checked );
}

void MainWindow::on_actionRotate_right_triggered( ) {
  editor->rotate( true );
}

void MainWindow::on_actionRotate_left_triggered( ) {
  editor->rotate( false );
}

bool MainWindow::open( const QString &fname ) {
  QFile fl( fname );
  if( !fl.exists( ) ) {
    QMessageBox::warning( this, tr( "Error!" ), tr( "File \"%1\" does not exists!" ).arg(
                            fname ), QMessageBox::Ok, QMessageBox::NoButton );
    std::cerr << tr( "Error: This file does not exists: " ).toStdString( ) << fname.toStdString( ) << std::endl;
    return( false );
  }
  if( fl.open( QFile::ReadOnly ) ) {
    QDataStream ds( &fl );
    setCurrentFile( QFileInfo( fname ) );
    try {
      editor->load( ds );
    }
    catch( std::runtime_error &e ) {
      std::cerr << tr( "Error loading project: " ).toStdString( ) << e.what( ) << std::endl;
      QMessageBox::warning( this, tr( "Error!" ), tr( "Could not open file.\nError: %1" ).arg(
                              e.what( ) ), QMessageBox::Ok, QMessageBox::NoButton );
      clear( );
      return( false );
    }
  }
  else {
    std::cerr << tr( "Could not open file in ReadOnly mode : " ).toStdString( ) << fname.toStdString( ) << "." <<
      std::endl;
    return( false );
  }
  fl.close( );

  rfController->addFile( fname );
  ui->statusBar->showMessage( tr( "File loaded successfully." ), 2000 );
  return( true );
}

void MainWindow::scrollView( int dx, int dy ) {
  ui->graphicsView->scroll( dx, dy );
}

void MainWindow::on_actionOpen_triggered( ) {
  QString fname =
    QFileDialog::getOpenFileName( this, tr( "Open File" ), defaultDirectory.absolutePath( ),
                                  tr( "Panda files (*.panda)" ) );
  if( fname.isEmpty( ) ) {
    return;
  }
  open( fname );
}

void MainWindow::on_actionSave_triggered( ) {
  save( );
}

void MainWindow::on_actionAbout_triggered( ) {
  QMessageBox::about( this, "wiRED Panda",
                      tr(
                        "<p>wiRED Panda is a software developed by the students of the Federal University of São Paulo."
                        " This project was created in order to help students to learn about logic circuits.</p>"
                        "<p>Software version: %1</p>"
                        "<p><strong>Creators:</strong></p>"
                        "<ul>"
                        "<li> Davi Morales </li>"
                        "<li> Héctor Castelli </li>"
                        "<li> Lucas Lellis </li>"
                        "<li> Rodrigo Torres </li>"
                        "<li> Prof. Fábio Cappabianco, Ph.D. </li>"
                        "</ul>"
                        "<p><a href=\"http://gibis-unifesp.github.io/wiRedPanda/\">Visit our website!</a></p>" )
                      .arg(
                        QApplication::applicationVersion( ) ) );
}

void MainWindow::on_actionAbout_Qt_triggered( ) {
  QMessageBox::aboutQt( this );
}

bool MainWindow::closeFile( ) {
  bool ok = true;
  if( !editor->getUndoStack( )->isClean( ) ) {
    int ret = confirmSave( );
    if( ret == QMessageBox::Save ) {
      ok = save( );
    }
    else if( ret == QMessageBox::Cancel ) {
      ok = false;
    }
  }
  return( ok );
}

void MainWindow::closeEvent( QCloseEvent *e ) {
#ifdef DEBUG
  return;
#endif
  if( closeFile( ) ) {
    close( );
  }
  else {
    e->ignore( );
  }
}

void MainWindow::on_actionSave_As_triggered( ) {
  QString fname = currentFile.absoluteFilePath( );
  QString path = defaultDirectory.absolutePath( );
  if( !currentFile.fileName( ).isEmpty( ) ) {
    path = currentFile.absoluteFilePath( );
  }
  fname =
    QFileDialog::getSaveFileName( this, tr( "Save File as ..." ), path, tr(
                                    "Panda files (*.panda)" ) );
  if( fname.isEmpty( ) ) {
    return;
  }
  if( !fname.endsWith( ".panda" ) ) {
    fname.append( ".panda" );
  }
  setCurrentFile( QFileInfo( fname ) );
  save( );
}

QFileInfo MainWindow::getCurrentFile( ) const {
  return( currentFile );
}

void MainWindow::setCurrentFile( const QFileInfo &value ) {
  qDebug( ) << "Setting current file to: " << value.absoluteFilePath( );
  currentFile = value;
  if( value.fileName( ).isEmpty( ) ) {
    setWindowTitle( "wiRED PANDA" );
  }
  else {
    setWindowTitle( QString( "wiRED PANDA ( %1 )" ).arg( value.fileName( ) ) );
  }
  rfController->addFile( value.absoluteFilePath( ) );
  GlobalProperties::currentFile = currentFile.absoluteFilePath( );
  if( currentFile.exists( ) ) {
    defaultDirectory = currentFile.dir( );
  }
  else {
    defaultDirectory = QDir::home( );
  }
}

void MainWindow::on_actionSelect_all_triggered( ) {
  editor->selectAll( );
}

void MainWindow::updateRecentBoxes( ) {
  ui->verticalLayout_4->removeItem( ui->verticalSpacer_BOX );
  while( QLayoutItem * item = ui->verticalLayout_4->takeAt( 0 ) ) {
    if( QWidget * widget = item->widget( ) ) {
      widget->deleteLater( );
    }
  }
  QStringList files = rboxController->getFiles( );
  for( auto file : files ) {
    QString name = QFileInfo( file ).baseName( ).toUpper( );
    QPixmap pixmap( QString::fromUtf8( ":/basic/box.png" ) );
    ListItemWidget *item = new ListItemWidget( pixmap, name, "label_box", file, this );
    ui->verticalLayout_4->addWidget( item );
  }
  ui->verticalLayout_4->addItem( ui->verticalSpacer_BOX );
}

QString MainWindow::getOpenBoxFile( ) {
  return( QFileDialog::getOpenFileName( this, tr( "Open File as Box" ), defaultDirectory.absolutePath( ), tr(
                                          "Panda files (*.panda)" ) ) );
}

void MainWindow::on_actionOpen_Box_triggered( ) {
  /* LOAD FILE AS BOX */
  QString fname = getOpenBoxFile( );
  if( fname.isEmpty( ) ) {
    return;
  }
  QFile fl( fname );
  if( !fl.exists( ) ) {
    std::cerr << tr( "Error: This file does not exists: " ).toStdString( ) << fname.toStdString( ) << std::endl;
    return;
  }
  if( fl.open( QFile::ReadOnly ) ) {
    QString name = QFileInfo( fname ).baseName( ).toUpper( );
    QPixmap pixmap( QString::fromUtf8( ":/basic/box.png" ) );
    ListItemWidget *item = new ListItemWidget( pixmap, name, "label_box", fname );
    ui->verticalLayout_4->removeItem( ui->verticalSpacer_BOX );
    ui->verticalLayout_4->addWidget( item );
    ui->verticalLayout_4->addItem( ui->verticalSpacer_BOX );
    item->getLabel( )->startDrag( );
  }
  else {
    std::cerr << tr( "Could not open file in ReadOnly mode : " ).toStdString( ) << fname.toStdString( ) << "." <<
      std::endl;
    return;
  }
  fl.close( );

  rboxController->addFile( fname );

  updateRecentBoxes( );

  ui->statusBar->showMessage( tr( "Loaded box sucessfully." ), 2000 );
}

void MainWindow::on_lineEdit_textChanged( const QString &text ) {
  ui->searchLayout->removeItem( ui->VSpacer );
  while( QLayoutItem * item = ui->searchLayout->takeAt( 0 ) ) {
    if( QWidget * widget = item->widget( ) ) {
      delete widget;
    }
  }
  firstResult = nullptr;
  if( text.isEmpty( ) ) {
    ui->searchScrollArea->hide( );
    ui->tabWidget->show( );
  }
  else {
    ui->searchScrollArea->show( );
    ui->tabWidget->hide( );
    QList< Label* > boxes = ui->tabWidget->findChildren< Label* >( "label_box" );
    QRegularExpression regex( QString( ".*%1.*" ).arg( text ), QRegularExpression::CaseInsensitiveOption );
    QList< Label* > searchResults;
    QRegularExpression regex2( QString( "^label_.*%1.*" ).arg( text ), QRegularExpression::CaseInsensitiveOption );
    searchResults.append( ui->tabWidget->findChildren< Label* >( regex2 ) );
    for( auto *box : boxes ) {
      if( regex.match( box->auxData( ) ).hasMatch( ) ) {
        searchResults.append( box );
      }
    }
    for( auto *label : searchResults ) {
      ListItemWidget *item = new ListItemWidget( *label->pixmap( ), label->property( "Name" ).toString( ),
                                                 label->objectName( ), label->auxData( ) );
      if( !firstResult ) {
        firstResult = item->getLabel( );
      }
      ui->searchLayout->addWidget( item );
    }
  }
  ui->searchLayout->addItem( ui->VSpacer );
}

void MainWindow::on_lineEdit_returnPressed( ) {
  if( firstResult ) {
    firstResult->startDrag( );
    ui->lineEdit->clear( );
  }
}


void MainWindow::resizeEvent( QResizeEvent* ) {
  editor->getScene( )->setSceneRect( editor->getScene( )->sceneRect( ).united( ui->graphicsView->rect( ) ) );
}

void MainWindow::on_actionReload_File_triggered( ) {
  if( currentFile.exists( ) ) {
    if( closeFile( ) ) {
      open( currentFile.absoluteFilePath( ) );
    }
  }
}

void MainWindow::on_actionGates_triggered( bool checked ) {
  ui->actionWires->setEnabled( checked );
  if( checked == false ) {
    editor->showWires( checked );
  }
  else {
    editor->showWires( ui->actionWires->isChecked( ) );
  }
  editor->showGates( checked );
}


bool MainWindow::ExportToArduino( QString fname ) {
  try {
    if( fname.isEmpty( ) ) {
      return( false );
    }
    QVector< GraphicElement* > elements = editor->getScene( )->getElements( );
    SimulationController *sc = editor->getSimulationController( );
    sc->stop( );
    if( elements.isEmpty( ) ) {
      return( false );
    }
    if( !fname.endsWith( ".ino" ) ) {
      fname.append( ".ino" );
    }
    elements = SimulationController::sortElements( elements );


    CodeGenerator arduino( QDir::home( ).absoluteFilePath( fname ), elements );
    arduino.generate( );
    sc->start( );
    ui->statusBar->showMessage( tr( "Arduino code successfully generated." ), 2000 );

    qDebug( ) << "Arduino code successfully generated.";
  }
  catch( std::runtime_error &e ) {
    QMessageBox::warning( this, tr( "Error" ), tr(
                            "<strong>Error while exporting to arduino code:</strong><br>%1" ).arg( e.what( ) ) );
    return( false );
  }

  return( true );
}

bool MainWindow::on_actionExport_to_Arduino_triggered( ) {

  QString fname = QFileDialog::getSaveFileName( this, tr( "Generate Arduino Code" ),
                                                defaultDirectory.absolutePath( ), tr(
                                                  "Arduino file (*.ino)" ) );

  return( ExportToArduino( fname ) );
}


#define ZOOMFAC 0.1
void MainWindow::on_actionZoom_in_triggered( ) {
/*  QPointF scenePos = editor->getMousePos(); */

/*  QPointF screenCtr = ui->graphicsView->rect().center(); */


  ui->graphicsView->setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
  double newScale = std::round( gvzoom->scaleFactor( ) * 10 ) / 10.0 + ZOOMFAC;
  if( newScale <= GraphicsViewZoom::maxZoom ) {
    gvzoom->setScaleFactor( newScale );
    zoomChanged( );
  }
}

void MainWindow::on_actionZoom_out_triggered( ) {
  double newScale = std::round( gvzoom->scaleFactor( ) * 10 ) / 10.0 - ZOOMFAC;
  if( newScale >= GraphicsViewZoom::minZoom ) {
    gvzoom->setScaleFactor( newScale );
    zoomChanged( );
  }
}

void MainWindow::on_actionReset_Zoom_triggered( ) {
  gvzoom->setScaleFactor( 1.0 );
}

void MainWindow::zoomChanged( ) {
  ui->actionZoom_in->setEnabled( gvzoom->scaleFactor( ) + ZOOMFAC <= gvzoom->maxZoom );
  ui->actionZoom_out->setEnabled( gvzoom->scaleFactor( ) - ZOOMFAC >= gvzoom->minZoom );
}

void MainWindow::updateRecentFileActions( ) {
  QStringList files = rfController->getFiles( );

  int numRecentFiles = qMin( files.size( ), ( int ) RecentFilesController::MaxRecentFiles );
  if( numRecentFiles > 0 ) {
    ui->menuRecent_files->setEnabled( true );
  }
  for( int i = 0; i < numRecentFiles; ++i ) {
    QString text = QString( "&%1 %2" ).arg( i + 1 ).arg( QFileInfo( files[ i ] ).fileName( ) );
    recentFileActs[ i ]->setText( text );
    recentFileActs[ i ]->setData( files[ i ] );
    recentFileActs[ i ]->setVisible( true );
  }
  for( int i = numRecentFiles; i < RecentFilesController::MaxRecentFiles; ++i ) {
    recentFileActs[ i ]->setVisible( false );
  }
}

void MainWindow::openRecentFile( ) {
  QAction *action = qobject_cast< QAction* >( sender( ) );
  if( action ) {
    QString fileName = action->data( ).toString( );
    open( fileName );
  }
}

void MainWindow::createRecentFileActions( ) {
  for( int i = 0; i < RecentFilesController::MaxRecentFiles; ++i ) {
    recentFileActs[ i ] = new QAction( this );
    recentFileActs[ i ]->setVisible( false );
    connect( recentFileActs[ i ], &QAction::triggered, this, &MainWindow::openRecentFile );
    ui->menuRecent_files->addAction( recentFileActs[ i ] );
  }
  updateRecentFileActions( );
  for( int i = 0; i < RecentFilesController::MaxRecentFiles; ++i ) {
    ui->menuRecent_files->addAction( recentFileActs[ i ] );
  }
}

void MainWindow::on_actionPrint_triggered( ) {

  QString pdfFile =
    QFileDialog::getSaveFileName( this, tr( "Export to PDF" ), defaultDirectory.absolutePath( ), tr(
                                    "PDF files (*.pdf)" ) );
  if( pdfFile.isEmpty( ) ) {
    return;
  }
  if( !pdfFile.toLower( ).endsWith( ".pdf" ) ) {
    pdfFile.append( ".pdf" );
  }
  QPrinter printer( QPrinter::HighResolution );
  printer.setPageSize( QPrinter::A4 );
  printer.setOrientation( QPrinter::Portrait );
  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setOutputFileName( pdfFile );


  QPainter p;
  if( !p.begin( &printer ) ) {

    QMessageBox::warning( this, tr( "ERROR" ), tr( "Could not print this circuit to PDF." ), QMessageBox::Ok );
    return;
  }
  editor->getScene( )->render( &p, QRectF( ), editor->getScene( )->itemsBoundingRect( ).adjusted( -64, -64, 64, 64 ) );
  p.end( );
}
