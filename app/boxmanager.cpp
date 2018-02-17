#include "boxfilehelper.h"
#include "boxmanager.h"
#include "boxprototype.h"
#include "mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <qfileinfo.h>

BoxManager*BoxManager::globalBoxManager = nullptr;

BoxManager::BoxManager( MainWindow *mainWindow, QObject *parent ) : QObject( parent ), m_mainWindow( mainWindow ) {
  if( globalBoxManager == nullptr ) {
    globalBoxManager = this;
  }
  connect( &m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &BoxManager::reloadFile );
}

BoxManager::~BoxManager( ) {
  clear( );
}

void BoxManager::loadFile( QString fname ) {
  QFileInfo finfo = BoxFileHelper::findFile( fname );
  m_fileWatcher.addPath( finfo.absoluteFilePath( ) );
  if( m_boxes.contains( finfo.baseName( ) ) ) {
    qDebug( ) << "BoxManager: Box already inserted: " << finfo.baseName( );
  }
  else {
    qDebug( ) << "BoxManager: Inserting Box: " << finfo.baseName( );
    BoxPrototype *prototype = new BoxPrototype( finfo.absoluteFilePath( ) );
    m_boxes.insert( finfo.baseName( ), prototype );
    prototype->reload( );
  }
}

void BoxManager::clear( ) {
  qDebug( ) << "BoxManager::Clear";
  QMap< QString, BoxPrototype* > boxes = m_boxes;
  m_boxes.clear( );
  for( auto it = boxes.begin( ); it != boxes.end( ); it++ ) {
    delete it.value( );
  }
  m_fileWatcher.removePaths( m_fileWatcher.files( ) );
}

BoxPrototype* BoxManager::getPrototype( QString fname ) {
  qDebug( ) << "get prototype of " << fname;
  QFileInfo finfo = BoxFileHelper::findFile( fname );
  if( !m_boxes.contains( finfo.baseName( ) ) ) {
    return( nullptr );
  }
  return( m_boxes[ finfo.baseName( ) ] );
}

BoxManager* BoxManager::instance( ) {
  return( globalBoxManager );
}

void BoxManager::reloadFile( QString fileName ) {
  QString bname = QFileInfo( fileName ).baseName( );
  m_fileWatcher.addPath( fileName );
  if( warnAboutFileChange( bname ) ) {
    if( m_boxes.contains( bname ) ) {
      try {
        m_boxes[ bname ]->reload( );
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
  QMessageBox msgBox;
  if( m_mainWindow ) {
    msgBox.setParent( m_mainWindow );
  }
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  msgBox.setText( tr( "The file %1 changed, do you want to reload?" ).arg( fileName ) );
  msgBox.setWindowModality( Qt::ApplicationModal );
  msgBox.setDefaultButton( QMessageBox::Yes );
  return( msgBox.exec( ) == QMessageBox::Yes );
}
