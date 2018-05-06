#include "boxfilehelper.h"
#include "boxnotfoundexception.h"
#include "globalproperties.h"

#include <iostream>
#include <QDebug>
#include <QDir>

QFileInfo BoxFileHelper::findFile( QString fname, QString parentFile ) {
  qDebug( ) << "Loading file: " << fname << ", parentFile: " << parentFile;
  QFileInfo fileInfo( fname );
  QString myFile = fileInfo.fileName( );
  if( !fileInfo.isFile( ) ) {
    fileInfo.setFile( QDir::current( ), fileInfo.fileName( ) );
    if( !fileInfo.isFile( ) ) {
      fileInfo.setFile( QFileInfo( parentFile ).absoluteDir( ), myFile );
      if( !fileInfo.isFile( ) ) {
        QFileInfo currentFile( GlobalProperties::currentFile );
        fileInfo.setFile( currentFile.absoluteDir( ), myFile );
        if( !fileInfo.isFile( ) ) {
          std::cerr << "Error: This file does not exists: " << fname.toStdString( ) << std::endl;
          throw( BoxNotFoundException( QString(
                                         "Box linked file \"%1\" could not be found!\n"
                                         "Do you want to find this file?" )
                                       .arg( fname ).toStdString( ), nullptr ) );
        }
      }
    }
  }
  return( fileInfo );
}

void BoxFileHelper::verifyRecursion( QString fname ) {
  Q_UNUSED( fname );
  // TODO BoxFileHelper::verifyRecursion
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
