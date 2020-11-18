#include "box.h"
#include "filehelper.h"
#include "boxmanager.h"
#include "boxnotfoundexception.h"
#include "boxprototype.h"
#include "mainwindow.h"
#include "qfileinfo.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

BoxManager*BoxManager::globalBoxManager = nullptr;

BoxManager::BoxManager( MainWindow *mainWindow, QObject *parent ) : QObject( parent ), mainWindow( mainWindow ) {
  if( globalBoxManager == nullptr ) {
    globalBoxManager = this;
  }
  connect( &fileWatcher, &QFileSystemWatcher::fileChanged, this, &BoxManager::reloadFile );
}

BoxManager::~BoxManager( ) {
  clear( );
  fprintf(stderr, "Removing box manager");
  if( globalBoxManager == this ) {
    globalBoxManager = nullptr;
  }
}

bool BoxManager::tryLoadFile( QString &fname, QString parentFile ) {
  try {
    loadFile( fname, parentFile );
  }
  catch( BoxNotFoundException &err ) {
    COMMENT( "BoxNotFoundException thrown: " << err.what( ), 0 );
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
        return( tryLoadFile( fname, parentFile ) );
      }
    }
  }
  return( true );
}

void BoxManager::loadFile( QString &fname, QString parentFile ) {
  QFileInfo finfo = FileHelper::findBoxFile( fname, parentFile );
  fname = finfo.filePath( );
  Q_ASSERT( finfo.exists( ) && finfo.isFile( ) );
  fileWatcher.addPath( finfo.absoluteFilePath( ) );
  if( boxes.contains( finfo.baseName( ) ) ) {
    COMMENT( "Box already inserted: " << finfo.baseName( ).toStdString( ), 0 );
  }
  else {
    COMMENT( "Inserting Box: " << finfo.baseName( ).toStdString( ), 0 );
    BoxPrototype *prototype = new BoxPrototype( finfo.absoluteFilePath( ) );
    prototype->reload( );
    boxes.insert( finfo.baseName( ), prototype );
  }
}

void BoxManager::clear( ) {
  COMMENT( "Clear boxmanager", 0 );
  QMap< QString, BoxPrototype* > boxes_aux = boxes;
  boxes.clear( );
  for( auto it : boxes_aux ) {
    delete it;
  }
  if( fileWatcher.files( ).size( ) > 0 ) {
    fileWatcher.removePaths( fileWatcher.files( ) );
  }
  COMMENT( "Finished clearing boxmanager.", 0 );
}

void BoxManager::updateRecentBoxes( const QString &fname ) {
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
  if( tryLoadFile( fname, parentFile ) ) {
    box->loadFile( fname );
  }
  updateRecentBoxes( fname );
  return( true );
}

BoxPrototype* BoxManager::getPrototype( QString fname ) {
  Q_ASSERT( !fname.isEmpty( ) );
  QFileInfo finfo( fname );
  if( !boxes.contains( finfo.baseName( ) ) ) {
    return( nullptr );
  }
  return( boxes[ finfo.baseName( ) ] );
}

bool BoxManager::updatePrototypeFilePathName( QString sourceName, QString targetName ) {
  COMMENT( "Updating box name from " << sourceName.toStdString( ) << " to " << targetName.toStdString( ), 0 );
  Q_ASSERT( !sourceName.isEmpty( ) );
  Q_ASSERT( !targetName.isEmpty( ) );
  QFileInfo finfo( sourceName );
  if( !boxes.contains( finfo.baseName( ) ) ) {
    return( false );
  }
  COMMENT( "Updating prototype box name.", 0 );
  auto proto = boxes[ finfo.baseName( ) ];
  proto->fileName( targetName );
  COMMENT( "Updating fileWatcher. Removing " << sourceName.toStdString( ), 0 );
  if( fileWatcher.removePath( sourceName ) ) {
    COMMENT( "Adding " << targetName.toStdString( ) << " to fileWatcher.", 0 );
    fileWatcher.addPath( targetName );
  }
  else {
    COMMENT( "Warning. FileWatcher did not exist. Probably already changed by other box instance update.", 0 );
  }
  return( true );
}

BoxManager* BoxManager::instance( ) {
  return( globalBoxManager );
}

void BoxManager::reloadFile( QString fileName ) {
  COMMENT( "Change in box " << fileName.toStdString( ) << " detected.", 0 );
  QString bname = QFileInfo( fileName ).baseName( );
  fileWatcher.addPath( fileName );
  if( warnAboutFileChange( bname ) ) {
    if( boxes.contains( bname ) ) {
      try {
        boxes[ bname ]->reload( );
      }
      catch( std::runtime_error &e ) {
        QMessageBox::warning( mainWindow, "Error", tr( "Error reloading Box: " ) + e.what( ), QMessageBox::Ok, QMessageBox::NoButton );
      }
    }
  }
  emit updatedBox( );
}

// Maybe this funcion should never be called and the main project should reload the box every time it changes.
bool BoxManager::warnAboutFileChange( const QString &fileName ) {
  COMMENT( "File " << fileName.toStdString( ) << " has changed!", 0 );
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
