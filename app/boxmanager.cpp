#include "boxfilehelper.h"
#include "boxmanager.h"
#include "boxprototype.h"
#include "mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <qfileinfo.h>

BoxManager*BoxManager::globalBoxManager = nullptr;

BoxManager::BoxManager( MainWindow *window, QObject *parent ) : QObject( parent ), mainWindow( window ) {
  if( globalBoxManager == nullptr ) {
    globalBoxManager = this;
  }
  // Warning: Parent can be null!
}

BoxManager::~BoxManager( ) {
  clear( );
}

void BoxManager::loadFile( QString fname ) {
  QFileInfo finfo = BoxFileHelper::findFile( fname );
  if( boxes.contains( finfo.absoluteFilePath( ) ) ) {
    qDebug( ) << "BoxManager: Box already inserted: " << finfo.baseName( );
    //FIXME: Missing implementation
  }
  else {
    qDebug( ) << "BoxManager: Inserting Box: " << finfo.baseName( );
    fileWatcher.addPath( finfo.absoluteFilePath( ) );
    boxes.insert( finfo.absoluteFilePath( ), new BoxPrototype( finfo.absoluteFilePath( ) ) );
  }
}

void BoxManager::clear( ) {
  qDebug( ) << "BoxManager::Clear";
  for( auto it = boxes.begin( ); it != boxes.end( ); it++ ) {
    delete it.value( );
  }
  boxes.clear( );
  fileWatcher.removePaths( fileWatcher.files( ) );
}

BoxManager* BoxManager::instance( ) {
  return( globalBoxManager );
}

void BoxManager::reloadFile( QString fname ) {
  if( warnAboutFileChange( fname ) ) {
    if( boxes.contains( fname ) ) {
      try {
        boxes[ fname ]->reload( );
      }
      catch( std::runtime_error &e ) {
        // TODO: Warn user reload didn't work
      }
    }
    // TODO DO something
  }
}

bool BoxManager::warnAboutFileChange( const QString &fileName ) {
  qDebug( ) << "File " << fileName << " has changed!";
  QFileInfo finfo( fileName );
  QMessageBox msgBox;
  if( mainWindow ) {
    msgBox.setParent( mainWindow );
  }
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  msgBox.setText( tr( "The file %1 changed, do you want to reload?" ).arg( finfo.fileName( ) ) );
  msgBox.setWindowModality( Qt::ApplicationModal );
  msgBox.setDefaultButton( QMessageBox::Yes );
  return( msgBox.exec( ) == QMessageBox::Yes );
}
