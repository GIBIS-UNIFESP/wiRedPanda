#include "ic.h"
#include "editor.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "serializationfunctions.h"

#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QMessageBox>
#include <stdexcept>
#include <QSaveFile>

bool SerializationFunctions::update( QString &fileName, QString dirName ) {
  double version;
  QRectF rect;
  QList< QGraphicsItem* > itemList;
  QFile file( fileName );
  if( file.open( QFile::ReadOnly ) ) {
    COMMENT( "Started reading IC file " << fileName.toStdString( ), 0 );
    QMap< quint64, QNEPort* > map = QMap< quint64, QNEPort* >( );
    QDataStream ds( &file );
    version = loadVersion( ds );
    loadDolphinFilename( ds, version );
    rect = loadRect( ds, version );
    COMMENT( "Version: " << version, 0 );
    COMMENT( "Element deserialization.", 0 );
    itemList = loadMoveData( dirName, ds, version, map );
    COMMENT( "Finished loading data", 0 );
  }
  QSaveFile fl( fileName );
  COMMENT( "Before saving data", 0 );
  if( fl.open( QFile::WriteOnly ) ) {
    COMMENT( "Start updating IC " << fileName.toStdString( ), 0 );
    QDataStream ds( &fl );
    ds << QApplication::applicationName( ) + " " + QString::number( GlobalProperties::version );
    ds << rect;
    COMMENT( "Element serialization.", 0 );
    serialize( itemList, ds );
  }
  if( !fl.commit( ) ) {
    std::cerr << "Could not save file: " + fl.errorString( ).toStdString( ) + "." << std::endl;
    return( false );
  }

  COMMENT( "Finished updating IC " << fileName.toStdString( ), 0 );
  return( true );
}

void SerializationFunctions::serialize( const QList< QGraphicsItem* > &items, QDataStream &ds ) {
  for( QGraphicsItem *item: items ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      ds << GraphicElement::Type;
      ds << static_cast< quint64 >( elm->elementType( ) );
      elm->save( ds );
    }
  }
  for( QGraphicsItem *item: items ) {
    if( item->type( ) == QNEConnection::Type ) {
      COMMENT( "Writing Connection.", 0 );
      QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
      ds << QNEConnection::Type;
      conn->save( ds );
    }
  }
}

QList< QGraphicsItem* > SerializationFunctions::deserialize( QDataStream &ds, double version, QString parentFile, QMap< quint64, QNEPort* > portMap ) {
  QList< QGraphicsItem* > itemList;
  while( !ds.atEnd( ) ) {
    int32_t type;
    ds >> type;
    COMMENT( "Type: " << type, 0 );
    if( type == GraphicElement::Type ) {
      quint64 elmType;
      ds >> elmType;
      COMMENT( "Building " << ElementFactory::typeToText( static_cast< ElementType >( elmType ) ).toStdString( ) << " element.", 0 );
      GraphicElement *elm = ElementFactory::buildElement( static_cast< ElementType >( elmType ) );
      if( elm ) {
        itemList.append( elm );
        elm->load( ds, portMap, version );
        if( elm->elementType( ) == ElementType::IC ) {
          COMMENT( "Loading IC.", 0 );
          IC *ic = qgraphicsitem_cast< IC* >( elm );
          ICManager::instance( )->loadIC( ic, ic->getFile( ), parentFile );
        }
        elm->setSelected( true );
      }
      else {
        throw( std::runtime_error( ERRORMSG( "Could not build element." ) ) );
      }
    }
    else if( type == QNEConnection::Type ) {
      COMMENT( "Reading Connection.", 0 );
      QNEConnection *conn = ElementFactory::buildConnection( );
      conn->setSelected( true );
      if( !conn->load( ds, portMap ) ) {
        COMMENT( "Deleting connection.", 0 );
        delete conn;
      }
      else {
        itemList.append( conn );
      }
    }
    else {
      qDebug( ) << type;
      throw( std::runtime_error( ERRORMSG( "Invalid type. Data is possibly corrupted." ) ) );
    }
  }
  return( itemList );
}

double SerializationFunctions::loadVersion( QDataStream &ds ) {
  COMMENT( "Loading version.", 0 );
  QString str;
  ds >> str;
  if( !str.startsWith( QApplication::applicationName( ) ) ) {
    throw( std::runtime_error( ERRORMSG( "Invalid file format." ) ) );
  }
  COMMENT( "String: " << str.toStdString( ), 0 );
  bool ok;
  double version = GlobalProperties::toDouble( str.split( " " ).at( 1 ), &ok );
  COMMENT( "Version: " << version, 0 );
  if( !ok ) {
    throw( std::runtime_error( ERRORMSG( "Invalid version number." ) ) );
  }
  return( version );
}

QString SerializationFunctions::loadDolphinFilename( QDataStream &ds, double version ) {
  QString str = "none";
  if( version >= 3.0 )
    ds >> str;
  return( str );
}

QRectF SerializationFunctions::loadRect( QDataStream &ds, double version ) {
  QRectF rect;
  if( version >= 1.4 )
    ds >> rect;
  return( rect );
}

QList< QGraphicsItem* > SerializationFunctions::loadMoveData( QString dirName, QDataStream &ds, double version, QMap< quint64, QNEPort* > portMap ) {
  QList< QGraphicsItem* > itemList;
  while( !ds.atEnd( ) ) {
    int type;
    ds >> type;
    COMMENT( "Type: " << type, 0 );
    if( type == GraphicElement::Type ) {
      quint64 elmType;
      ds >> elmType;
      COMMENT( "Building " << ElementFactory::typeToText( static_cast< ElementType >( elmType ) ).toStdString( ) << " element.", 0 );
      GraphicElement *elm = ElementFactory::buildElement( static_cast< ElementType >( elmType ) );
      if( elm ) {
        itemList.append( elm );
        elm->load( ds, portMap, version );
        if( elm->elementType( ) == ElementType::IC ) {
          IC *ic = qgraphicsitem_cast< IC* >( elm );
          QString oldName = ic->getFile( );
          QString newName = dirName + "/boxes/" + QFileInfo( oldName ).fileName( );
          ic->setFile( newName );
        }
        elm->updateSkinsPath( dirName + "/skins/" );
      }
      else {
        throw( std::runtime_error( ERRORMSG( "Could not build element." ) ) );
      }
    }
    else if( type == QNEConnection::Type ) {
      COMMENT( "Reading Connection.", 0 );
      QNEConnection *conn = ElementFactory::buildConnection( );
      if( !conn->load( ds, portMap ) ) {
        COMMENT( "Deleting connection.", 0 );
        delete conn;
      }
      else {
        itemList.append( conn );
      }
    }
    else {
      qDebug( ) << type;
      throw( std::runtime_error( ERRORMSG( "Invalid type. Data is possibly corrupted." ) ) );
    }
  }
  COMMENT( "Finished loading data.", 0 );
  return( itemList );
}

QList< QGraphicsItem* > SerializationFunctions::load( QDataStream &ds, QString parentFile ) {
  COMMENT( "Started loading file.", 0 );
  QString str;
  ds >> str;
  if( !str.startsWith( QApplication::applicationName( ) ) ) {
    throw( std::runtime_error( ERRORMSG( "Invalid file format." ) ) );
  }
  bool ok;
  double version = GlobalProperties::toDouble( str.split( " " ).at( 1 ), &ok );
  if( !ok ) {
    throw( std::runtime_error( ERRORMSG( "Invalid version number." ) ) );
  }
  loadDolphinFilename( ds, version );
  loadRect( ds, version );
  COMMENT( "Header Ok. Version: " << version, 0 );
  QList< QGraphicsItem* > items = deserialize( ds, version, parentFile );
  COMMENT( "Finished reading items.", 0 );
  return( items );
}
