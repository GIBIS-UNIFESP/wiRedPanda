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
  // Warning: Parent can be null!
}

BoxManager::~BoxManager( ) {
  clear( );
}

void BoxManager::loadFile( QString fname ) {
  QFileInfo finfo = BoxFileHelper::findFile( fname );
  if( m_boxes.contains( finfo.baseName( ) ) ) {
    qDebug( ) << "BoxManager: Box already inserted: " << finfo.baseName( );
    //FIXME: Missing implementation
  }
  else {
    qDebug( ) << "BoxManager: Inserting Box: " << finfo.baseName( );
    m_fileWatcher.addPath( finfo.absoluteFilePath( ) );
    BoxPrototype *prototype = new BoxPrototype( finfo.absoluteFilePath( ) );
    m_boxes.insert( finfo.baseName( ), prototype );
    prototype->reload( );
  }
}

void BoxManager::clear( ) {
  qDebug( ) << "BoxManager::Clear";
  for( auto it = m_boxes.begin( ); it != m_boxes.end( ); it++ ) {
    delete it.value( );
  }
  m_boxes.clear( );
  m_fileWatcher.removePaths( m_fileWatcher.files( ) );
}

BoxPrototype* BoxManager::getPrototype( QString fname ) {
  QFileInfo finfo = BoxFileHelper::findFile( fname );
  return( m_boxes[ finfo.baseName( ) ] );
}

BoxManager* BoxManager::instance( ) {
  return( globalBoxManager );
}

void BoxManager::reloadFile( QString fname ) {
  if( warnAboutFileChange( fname ) ) {
    if( m_boxes.contains( fname ) ) {
      try {
        m_boxes[ fname ]->reload( );
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
  if( m_mainWindow ) {
    msgBox.setParent( m_mainWindow );
  }
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  msgBox.setText( tr( "The file %1 changed, do you want to reload?" ).arg( finfo.fileName( ) ) );
  msgBox.setWindowModality( Qt::ApplicationModal );
  msgBox.setDefaultButton( QMessageBox::Yes );
  return( msgBox.exec( ) == QMessageBox::Yes );
}
