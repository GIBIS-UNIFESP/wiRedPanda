#include "boxfilehelper.h"
#include "boxnotfoundexception.h"
#include "globalproperties.h"

#include <QDir>
#include <iostream>

QFileInfo BoxFileHelper::findFile( QString fname ) {
  QFileInfo fileInfo( fname );
  QString myFile = fileInfo.fileName( );
  if( !fileInfo.exists( ) ) {
    fileInfo.setFile( QDir::current( ), fileInfo.fileName( ) );
//    if( !fileInfo.exists( ) ) { // TODO
//      fileInfo.setFile( QFileInfo( parentFile ).absoluteDir( ), myFile );
    if( !fileInfo.exists( ) ) {
      QFileInfo currentFile( GlobalProperties::currentFile );
      fileInfo.setFile( currentFile.absoluteDir( ), myFile );
      if( !fileInfo.exists( ) ) {
        std::cerr << "Error: This file does not exists: " << fname.toStdString( ) << std::endl;
        throw( BoxNotFoundException( QString(
                                       "Box linked file \"%1\" could not be found!\n"
                                       "Do you want to find this file?" )
                                     .arg( fname ).toStdString( ), nullptr ) );
      }
    }
//    }
  }
  return( fileInfo );
}

QString BoxFileHelper::verifyRecursion( QString fname ) {
  // TODO
//  std::string msg = "Oh no! I'm my own parent.\nSomething is not ok...";
//  if( !parentFile.isEmpty( ) && ( fname == parentFile ) ) {
//    throw( std::runtime_error( msg ) );
//  }
//  for( Box *box = parentBox; box != nullptr; box = box->getParentBox( ) ) {
////    qDebug( ) << "File: " << box->getFile( );
//    if( box->getFile( ) == fname ) {
//      throw( std::runtime_error( msg ) );
//    }
//  }
}
