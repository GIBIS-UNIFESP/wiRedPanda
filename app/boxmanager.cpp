#include "boxfilehelper.h"
#include "boxmanager.h"
#include "boxprototype.h"

#include <QDebug>
#include <qfileinfo.h>

BoxManager::BoxManager( MainWindow *window, QObject *parent ) : QObject( parent ), mainWindow( window ) {
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
  //TODO Verify if box is used anywhere before asking
  //FIXME: Unimplemented yet
  return( true );
}
