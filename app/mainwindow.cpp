#include "globalproperties.h"
#include "listitemwidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QShortcut>
#include <iostream>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
  ui->setupUi( this );
  editor = new Editor( this );
  ui->graphicsView->setScene( editor->getScene( ) );
/*  ui->graphicsView->setBackgroundBrush(QBrush(QColor(Qt::gray))); */
  ui->graphicsView->setRenderHint( QPainter::Antialiasing, true );
  ui->graphicsView->setAcceptDrops( true );
  editor->setElementEditor( ui->widgetElementEditor );
  ui->searchScrollArea->hide( );
  setCurrentFile( QFileInfo( ) );
#ifdef DEBUG
  createUndoView( );
#endif
/*  connect( ui->actionUndo, &QAction::triggered, editor->getUndoStack( ), &QUndoStack::undo ); */
/*  connect( ui->actionRedo, &QAction::triggered, editor->getUndoStack( ), &QUndoStack::redo ); */

  undoAction = editor->getUndoStack( )->createUndoAction( this, tr( "&Undo" ) );
  undoAction->setShortcuts( QKeySequence::Undo );

  redoAction = editor->getUndoStack( )->createRedoAction( this, tr( "&Redo" ) );
  redoAction->setShortcuts( QKeySequence::Redo );

  ui->menuEdit->insertAction( ui->actionUndo, undoAction );
  ui->menuEdit->insertAction( ui->actionRedo, redoAction );
  ui->menuEdit->removeAction( ui->actionUndo );
  ui->menuEdit->removeAction( ui->actionRedo );

  connect( editor, &Editor::scroll, this, &MainWindow::scrollView );

  QShortcut *shortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ), this );
  connect( shortcut, SIGNAL( activated( ) ), ui->lineEdit, SLOT( setFocus( ) ) );
  ui->graphicsView->setCacheMode( QGraphicsView::CacheBackground );
  firstResult = nullptr;
  updateRecentBoxes( );
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

bool MainWindow::save( ) {
  QString fname = currentFile.absoluteFilePath( );
  if( !currentFile.exists( ) ) {
    fname =
      QFileDialog::getSaveFileName( this, tr( "Save File" ), defaultDirectory.absolutePath( ), tr(
                                      "Panda files (*.panda)" ) );
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
      std::cerr << "Error saving project: " << e.what( ) << std::endl;
      return( false );
    }
  }
  else {
    std::cerr << "Could not open file in WriteOnly mode : " << fname.toStdString( ) << "." << std::endl;
    return( false );
  }
  fl.flush( );
  fl.close( );
  setCurrentFile( QFileInfo( fname ) );
  ui->statusBar->showMessage( "Saved file sucessfully.", 2000 );
  editor->getUndoStack( )->setClean( );
  qDebug( ) << "Saved file successfully!";
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
  if( !editor->getUndoStack( )->isClean( ) ) {
    int ret = confirmSave( );
    if( ret == QMessageBox::Save ) {
      if( save( ) ) {
        clear( );
      }
    }
    else if( ret == QMessageBox::Discard ) {
      clear( );
    }
    else if( ret == QMessageBox::Cancel ) {
      return;
    }
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

void MainWindow::open( const QString &fname ) {
  QFile fl( fname );
  if( !fl.exists( ) ) {
    QMessageBox::warning( this, "Error!", QString( "File \"%1\" does not exists!" ).arg(
                            fname ), QMessageBox::Ok, QMessageBox::NoButton );
    std::cerr << "Error: This file does not exists: " << fname.toStdString( ) << std::endl;
    return;
  }
  if( fl.open( QFile::ReadOnly ) ) {
    QDataStream ds( &fl );
    setCurrentFile( QFileInfo( fname ) );
    try {
      editor->load( ds );
    }
    catch( std::runtime_error &e ) {
      std::cerr << "Error loading project: " << e.what( ) << std::endl;
      QMessageBox::warning( this, "Error!", "Could not open file.\nError: " + QString(
                              e.what( ) ), QMessageBox::Ok, QMessageBox::NoButton );
      clear( );
      return;
    }
  }
  else {
    std::cerr << "Could not open file in ReadOnly mode : " << fname.toStdString( ) << "." << std::endl;
    return;
  }
  fl.close( );
  ui->statusBar->showMessage( "Loaded file sucessfully.", 2000 );
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
  QMessageBox::about( this, "Wired Panda",
                      tr(
                        "Wired Panda is a software built to help students to learn about logic circuits.\nVersion: %1\n\nCreators:\nDavi Morales\nHéctor Castelli\nLucas Lellis\nRodrigo Torres\nSupervised by: Fábio Cappabianco." ).arg(
                        QApplication::applicationVersion( ) ) );
}

void MainWindow::on_actionAbout_Qt_triggered( ) {
  QMessageBox::aboutQt( this );
}

void MainWindow::on_actionDelete_triggered( ) {
  editor->deleteElements( );
}

void MainWindow::on_lineEdit_textEdited( const QString & ) {

}

void MainWindow::closeEvent( QCloseEvent *e ) {
#ifdef DEBUG
  return;
#endif
  if( !editor->getUndoStack( )->isClean( ) ) {
    int ret = confirmSave( );
    if( ret == QMessageBox::Save ) {
      if( save( ) ) {
        close( );
      }
    }
    else if( ret == QMessageBox::Discard ) {
      close( );
    }
    else if( ret == QMessageBox::Cancel ) {
      e->ignore( );
    }
  }
}

void MainWindow::on_actionCopy_triggered( ) {
  QClipboard *clipboard = QApplication::clipboard( );
  QMimeData *mimeData = new QMimeData;
  QByteArray itemData;
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  editor->copy( dataStream );
  mimeData->setData( "application/copydata", itemData );
  clipboard->setMimeData( mimeData );
}

void MainWindow::on_actionCut_triggered( ) {
  QClipboard *clipboard = QApplication::clipboard( );
  QMimeData *mimeData = new QMimeData;
  QByteArray itemData;
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  editor->cut( dataStream );
  mimeData->setData( "application/copydata", itemData );
  clipboard->setMimeData( mimeData );
}

void MainWindow::on_actionPaste_triggered( ) {
  const QClipboard *clipboard = QApplication::clipboard( );
  const QMimeData *mimeData = clipboard->mimeData( );
  if( mimeData->hasFormat( "application/copydata" ) ) {
    QByteArray itemData = mimeData->data( "application/copydata" );
    QDataStream dataStream( &itemData, QIODevice::ReadOnly );
    editor->paste( dataStream );
  }
}

void MainWindow::on_actionSave_As_triggered( ) {
  QString fname = currentFile.absoluteFilePath( );
  fname =
    QFileDialog::getSaveFileName( this, tr( "Save File as ..." ), defaultDirectory.absolutePath( ), tr(
                                    "Panda files (*.panda)" ) );
  if( fname.isEmpty( ) ) {
    return;
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
      std::cerr << "Error saving project: " << e.what( ) << std::endl;
      return;
    }
  }
  else {
    std::cerr << "Could not open file in WriteOnly mode : " << fname.toStdString( ) << "." << std::endl;
    return;
  }
  fl.close( );
  ui->statusBar->showMessage( "Saved file sucessfully.", 2000 );
  qDebug( ) << "Saving as ... ";
  setCurrentFile( QFileInfo( fname ) );
}

QFileInfo MainWindow::getCurrentFile( ) const {
  return( currentFile );
}

void MainWindow::setCurrentFile( const QFileInfo &value ) {
  qDebug( ) << "Setting current file to: " << value.absoluteFilePath( );
  currentFile = value;
  if( value.fileName( ).isEmpty( ) ) {
    setWindowTitle( "WiRED PANDA" );
  }
  else {
    setWindowTitle( QString( "WiRED PANDA ( %1 )" ).arg( value.fileName( ) ) );
  }
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
      delete widget;
    }
  }
  QSettings settings;
  QStringList files = settings.value( "recentBoxes" ).toStringList( );
  foreach( QString file, files ) {
    QString name = QFileInfo( file ).baseName( ).toUpper( );
    QPixmap pixmap( QString::fromUtf8( ":/basic/box.png" ) );
    ListItemWidget *item = new ListItemWidget( pixmap, name, "label_box", file );
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
    std::cerr << "Error: This file does not exists: " << fname.toStdString( ) << std::endl;
    return;
  }
  if( fl.open( QFile::ReadOnly ) ) {
    QString name = QFileInfo( fname ).baseName( ).toUpper( );
    QPixmap pixmap( QString::fromUtf8( ":/basic/box.png" ) );
    ListItemWidget *item = new ListItemWidget( pixmap, name, "label_box", fname );
    ui->verticalLayout_4->removeItem( ui->verticalSpacer_BOX );
    ui->verticalLayout_4->addWidget( item );
    ui->verticalLayout_4->addItem( ui->verticalSpacer_BOX );
    item->getLabel()->startDrag();
  }
  else {
    std::cerr << "Could not open file in ReadOnly mode : " << fname.toStdString( ) << "." << std::endl;
    return;
  }
  fl.close( );

  QSettings settings;
  QStringList files = settings.value( "recentBoxes" ).toStringList( );
  files.removeAll( fname );
  files.prepend( fname );
  while( files.size( ) > 5 ) {
    files.removeLast( );
  }
  settings.setValue( "recentBoxes", files );

  updateRecentBoxes( );

  ui->statusBar->showMessage( "Loaded box sucessfully.", 2000 );
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
    QRegularExpression regex( QString( ".*%1.*" ).arg( text ) );
    QList< Label* > searchResults;
    foreach( Label * box, boxes ) {
      if( regex.match( box->auxData( ) ).hasMatch( ) ) {
        searchResults.append( box );
      }
    }
    searchResults.append( ui->tabWidget->findChildren< Label* >( QRegularExpression( QString( "^label_.*%1.*" ).arg(
                                                                                       text ) ) ) );
    foreach( Label * label, searchResults ) {
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
