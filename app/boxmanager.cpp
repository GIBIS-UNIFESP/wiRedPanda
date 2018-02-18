#include "box.h"
#include "boxfilehelper.h"
#include "boxmanager.h"
#include "boxnotfoundexception.h"
#include "boxprototype.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <qfileinfo.h>

BoxManager*BoxManager::globalBoxManager = nullptr;

BoxManager::BoxManager( MainWindow *mainWindow, QObject *parent ) : QObject( parent ), mainWindow( mainWindow ) {
  if( globalBoxManager == nullptr ) {
    globalBoxManager = this;
  }
  connect( &fileWatcher, &QFileSystemWatcher::fileChanged, this, &BoxManager::reloadFile );
}

BoxManager::~BoxManager( ) {
  clear( );
}

void BoxManager::loadFile( QString fname, QString parentFile ) {
  QFileInfo finfo = BoxFileHelper::findFile( fname, parentFile );
  fileWatcher.addPath( finfo.absoluteFilePath( ) );
  if( boxes.contains( finfo.baseName( ) ) ) {
    qDebug( ) << "BoxManager: Box already inserted: " << finfo.baseName( );
  }
  else {
    qDebug( ) << "BoxManager: Inserting Box: " << finfo.baseName( );
    BoxPrototype *prototype = new BoxPrototype( finfo.absoluteFilePath( ) );
    boxes.insert( finfo.baseName( ), prototype );
    prototype->reload( );
  }
}

void BoxManager::clear( ) {
  qDebug( ) << "BoxManager::Clear";
  QMap< QString, BoxPrototype* > boxes_aux = boxes;
  boxes.clear( );
  for( auto it = boxes_aux.begin( ); it != boxes_aux.end( ); it++ ) {
    delete it.value( );
  }
  fileWatcher.removePaths( fileWatcher.files( ) );
}

void BoxManager::updateRecentBoxes( QString fname ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  QStringList files;
  if( settings.contains( "recentBoxes" ) ) {
    files = settings.value( "recentBoxes" ).toStringList( );
    files.removeAll( fname );
  }
  files.prepend( fname );
  settings.setValue( "recentBoxes", files );
  if( mainWindow ) {
    mainWindow->updateRecentBoxes( );
  }
}

bool BoxManager::loadBox( Box *box, QString fname, QString parentFile ) {
  try {
    loadFile( fname, parentFile );
    box->loadFile( fname );
  }
  catch( BoxNotFoundException &err ) {
    qDebug( ) << "BoxNotFoundException thrown: " << err.what( );
    int ret = QMessageBox::warning( mainWindow, tr( "Error" ), QString::fromStdString(
                                      err.what( ) ), QMessageBox::Ok, QMessageBox::Cancel );
    if( ret == QMessageBox::Cancel ) {
      return( false );
    }
    else {
      fname = mainWindow->getOpenBoxFile( );
      if( fname.isEmpty( ) ) {
        return( false );
      }
      else {
        return( loadBox( err.getBox( ), fname, parentFile ) );
      }
    }
  }

  updateRecentBoxes( fname );
  return( true );
}

BoxPrototype* BoxManager::getPrototype( QString fname ) {
  qDebug( ) << "get prototype of " << fname;
  QFileInfo finfo( fname );
  if( !boxes.contains( finfo.baseName( ) ) ) {
    return( nullptr );
  }
  return( boxes[ finfo.baseName( ) ] );
}

BoxManager* BoxManager::instance( ) {
  return( globalBoxManager );
}

void BoxManager::reloadFile( QString fileName ) {
  QString bname = QFileInfo( fileName ).baseName( );
  fileWatcher.addPath( fileName );
  if( warnAboutFileChange( bname ) ) {
    if( boxes.contains( bname ) ) {
      try {
        boxes[ bname ]->reload( );
      }
      catch( std::runtime_error &e ) {
        QMessageBox::warning( mainWindow, "Error", tr( "Error reloading Box: " ) + e.what( ),
                              QMessageBox::Ok, QMessageBox::NoButton );

      }
    }
  }
}

bool BoxManager::warnAboutFileChange( const QString &fileName ) {
  qDebug( ) << "File " << fileName << " has changed!";
  QMessageBox msgBox;
  if( mainWindow ) {
    msgBox.setParent( mainWindow );
  }
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  msgBox.setText( tr( "The file %1 changed, do you want to reload?" ).arg( fileName ) );
  msgBox.setWindowModality( Qt::ApplicationModal );
  msgBox.setDefaultButton( QMessageBox::Yes );
  return( msgBox.exec( ) == QMessageBox::Yes );
}
