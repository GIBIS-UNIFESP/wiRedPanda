#include "globalproperties.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMimeData>
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
  editor->getScene( )->setSceneRect( ui->graphicsView->rect( ) );
  ui->searchScrollArea->hide( );
  setCurrentFile( QFileInfo( ) );
#ifdef DEBUG
  createUndoView( );
#endif

  connect( ui->actionUndo, &QAction::triggered, editor->getUndoStack( ), &QUndoStack::undo );
  connect( ui->actionRedo, &QAction::triggered, editor->getUndoStack( ), &QUndoStack::redo );

  connect( editor, &Editor::scroll, this, &MainWindow::scrollView );

  QShortcut *shortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ), this );
  connect( shortcut, SIGNAL( activated( ) ), ui->lineEdit, SLOT( setFocus( ) ) );
/*
 *  ui->tabWidget->setTabEnabled(2,false);
 *  ui->tabWidget->setTabEnabled(3,false);
 */
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
  fl.close( );
  setCurrentFile( QFileInfo( fname ) );
  ui->statusBar->showMessage( "Saved file sucessfully.", 2000 );
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

void MainWindow::on_actionNew_triggered( ) {
  QMessageBox msgBox;
  msgBox.setParent( this );
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
  msgBox.setText( tr( "Do you want to save your changes?" ) );
  msgBox.setWindowModality( Qt::WindowModal );
  msgBox.setDefaultButton( QMessageBox::Save );
  int ret = msgBox.exec( );
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
  QMessageBox msgBox;
  msgBox.setParent( this );
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
  msgBox.setText( tr( "Do you want to save your changes?" ) );
  msgBox.setWindowModality( Qt::WindowModal );
  msgBox.setDefaultButton( QMessageBox::Save );
  int ret = msgBox.exec( );
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

void MainWindow::on_actionOpen_Box_triggered( ) {
  QString fname =
    QFileDialog::getOpenFileName( this, tr( "Open File as Box" ), defaultDirectory.absolutePath( ), tr(
                                    "Panda files (*.panda)" ) );
  if( fname.isEmpty( ) ) {
    return;
  }
  QFile fl( fname );
  if( !fl.exists( ) ) {
    std::cerr << "Error: This file does not exists: " << fname.toStdString( ) << std::endl;
    return;
  }
  if( fl.open( QFile::ReadOnly ) ) {
    try {
      QHBoxLayout *horizontalLayout_BOX = new QHBoxLayout( );
      horizontalLayout_BOX->setSpacing( 6 );
      horizontalLayout_BOX->setObjectName( QStringLiteral( "horizontalLayout_BOX" ) );
      horizontalLayout_BOX->setSizeConstraint( QLayout::SetFixedSize );
      QPixmap pixmap( QString::fromUtf8( ":/basic/box.png" ) );
      Label *label_BOX_ICON = new Label( ui->scrollAreaWidgetContents_Box );
      label_BOX_ICON->setObjectName( QStringLiteral( "label_box" ) );
      label_BOX_ICON->setPixmap( pixmap );
      label_BOX_ICON->setAuxData( fname );

      horizontalLayout_BOX->addWidget( label_BOX_ICON );

      QLabel *label_BOX = new QLabel( ui->scrollAreaWidgetContents_Box );
      label_BOX->setObjectName( QStringLiteral( "label_BOX" ) );
      label_BOX->setText( QFileInfo( fname ).baseName( ).toUpper( ) );
      horizontalLayout_BOX->addWidget( label_BOX );
      ui->verticalLayout_4->removeItem( ui->verticalSpacer_BOX );
      ui->verticalLayout_4->addLayout( horizontalLayout_BOX );
      ui->verticalLayout_4->addItem( ui->verticalSpacer_BOX );
    }
    catch( std::runtime_error &e ) {
      std::cerr << "Error loading project as box: " << e.what( ) << std::endl;
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
  ui->statusBar->showMessage( "Loaded box sucessfully.", 2000 );
}

void MainWindow::on_lineEdit_textChanged( const QString &text ) {
  ui->searchLayout->removeItem( ui->VSpacer );
  while (QLayoutItem* item = ui->searchLayout->takeAt(0)){
    if (QWidget* widget = item->widget())
      delete widget;
  }
  if( text.isEmpty( ) ) {
    ui->searchScrollArea->hide( );
    ui->tabWidget->show( );
  }
  else {
    ui->searchScrollArea->show( );
    ui->tabWidget->hide( );
    QList< Label* > searchResults =
      ui->tabWidget->findChildren< Label* >( QRegularExpression( QString( "^label_.*%1.*" ).arg( text ) ) );
    foreach( Label * label, searchResults ) {
      QHBoxLayout *itemLayout = new QHBoxLayout( );
      itemLayout->setSpacing( 6 );
      itemLayout->setObjectName( QStringLiteral( "itemLayout" ) );
      itemLayout->setSizeConstraint( QLayout::SetFixedSize );

      QFrame *searchResult = new QFrame( );
      searchResult->setObjectName( "searchResult" );
      searchResult->setLayout( itemLayout );

      Label *copy = new Label( this );
      copy->setObjectName( label->objectName( ) );
      copy->setPixmap( *label->pixmap( ) );

      QString name = label->objectName( ).remove( "label_" ).toUpper( );
      QLabel *nameLabel = new QLabel( name );

      itemLayout->addWidget( copy );
      itemLayout->addWidget( nameLabel );

      ui->searchLayout->addWidget( searchResult );
    }
  }
  ui->searchLayout->addItem( ui->VSpacer );
}
