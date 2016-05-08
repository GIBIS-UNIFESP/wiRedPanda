#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

#include "recentfilescontroller.h"

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
  for( int i = 0; i < files.size( ); ) {
    QFileInfo fileInfo( files.at( i ) );
    if( !fileInfo.exists( ) ) {
      files.removeAt( i );
    }
    else {
      ++i;
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
  if( !settings.contains( attrName ) ) {
    return( QStringList() );
  }
  QStringList files = settings.value( attrName ).toStringList( );
  for( int i = 0; i < files.size( ); ) {
    QFileInfo fileInfo( files.at( i ) );
    if( !fileInfo.exists( ) ) {
      files.removeAt( i );
    }
    else {
      ++i;
    }
  }
  while( files.size( ) > MaxRecentFiles ) {
    files.removeLast( );
  }
  settings.setValue( attrName, files );
  return( files );
}
