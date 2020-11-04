#include "box.h"
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

void SerializationFunctions::serialize( const QList< QGraphicsItem* > &items, QDataStream &ds ) {
  for( QGraphicsItem *item: items ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      ds << item->type( );
      ds << static_cast< quint64 >( elm->elementType( ) );
      elm->save( ds );
    }
  }
  for( QGraphicsItem *item: items ) {
    if( item->type( ) == QNEConnection::Type ) {
      QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
      ds << item->type( );
      conn->save( ds );
    }
  }
}

QList< QGraphicsItem* > SerializationFunctions::deserialize( QDataStream &ds, double version, QString parentFile,
                                                             QMap< quint64, QNEPort* > portMap ) {
  QList< QGraphicsItem* > itemList;
  while( !ds.atEnd( ) ) {
    int type;
    ds >> type;
    if( type == GraphicElement::Type ) {
      quint64 elmType;
      ds >> elmType;
      COMMENT( "Building " << ElementFactory::typeToText(
                 static_cast< ElementType >( elmType ) ).toStdString( ) << " element.",
               4 );
      GraphicElement *elm = ElementFactory::buildElement( static_cast< ElementType >( elmType ) );
      if( elm ) {
        itemList.append( elm );
        elm->load( ds, portMap, version );
        if( elm->elementType( ) == ElementType::BOX ) {
          Box *box = qgraphicsitem_cast< Box* >( elm );
          BoxManager::instance( )->loadBox( box, box->getFile( ), parentFile );
        }
        elm->setSelected( true );
      }
      else {
        throw( std::runtime_error( ERRORMSG( "Could not build element." ) ) );
      }
    }
    else if( type == QNEConnection::Type ) {
      QNEConnection *conn = ElementFactory::buildConnection( );
      conn->setSelected( true );
      if( !conn->load( ds, portMap ) ) {
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


QList< QGraphicsItem* > SerializationFunctions::load( QDataStream &ds, QString parentFile, Scene *scene ) {
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
  QRectF rect;
  if( version >= 1.4 ) {
    ds >> rect;
  }
  QList< QGraphicsItem* > items = deserialize( ds, version, parentFile );
  if( scene ) {
    for( QGraphicsItem *item : items ) {
      scene->addItem( item );
    }
    scene->setSceneRect( scene->itemsBoundingRect( ) );
    if( !scene->views( ).empty( ) ) {
      auto const scene_views = scene->views( );
      QGraphicsView *view = scene_views.first( );
      rect = rect.united( view->rect( ) );
      rect.moveCenter( QPointF( 0, 0 ) );
      scene->setSceneRect( scene->sceneRect( ).united( rect ) );
      view->centerOn( scene->itemsBoundingRect( ).center( ) );
    }
  }
  return( items );
}
