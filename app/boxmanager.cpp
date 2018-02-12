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
  QFileInfo finfo( fname );
  if( boxes.contains( finfo.absoluteFilePath( ) ) ) {
    //FIXME: Missing implementation
    qDebug( ) << "BoxManager: Box already inserted: " << fname;
  }
  else {
    qDebug( ) << "BoxManager: Inserting Box: " << fname;
    fileWatcher.addPath( finfo.absoluteFilePath( ) );
    boxes.insert( finfo.absoluteFilePath( ), new BoxPrototype( fname ) );
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
      boxes[ fname ]->reload( );
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
