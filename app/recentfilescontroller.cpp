#include "recentfilescontroller.h"

#include <QDebug>
#include <QFileInfo>
#include <qfile.h>
#include <qsettings.h>

RecentFilesController::RecentFilesController( QString attrName, QObject *parent ) : QObject( parent ) {
  this->attrName = attrName;
}

void RecentFilesController::addFile( QString fname ) {
  qDebug( ) << "Setting recent file to : \"" << fname << "\"";
  if( !QFile( fname ).exists( ) ) {
    return;
  }
  QSettings settings;
  QStringList files = settings.value( attrName ).toStringList( );

  files.removeAll( fname );

  files.prepend( fname );
  for( auto file : files ) {
    QFileInfo fileInfo( file );
    if( !fileInfo.exists( ) ) {
      files.removeAll( file );
    }
  }
  while( files.size( ) > MaxRecentFiles ) {
    files.removeLast( );
  }
  settings.setValue( attrName, files );

  emit recentFilesUpdated( );
}

QStringList RecentFilesController::getFiles( ) {
  QSettings settings;
  QStringList files = settings.value( attrName ).toStringList( );
  for( auto file : files ) {
    QFileInfo fileInfo( file );
    if( !fileInfo.exists( ) ) {
      files.removeAll( file );
    }
  }
  while( files.size( ) > MaxRecentFiles ) {
    files.removeLast( );
  }
  settings.setValue( attrName, files );
  return( files );
}
