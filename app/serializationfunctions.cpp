#include "box.h"
#include "editor.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "serializationfunctions.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QMessageBox>
#include <stdexcept>

void SerializationFunctions::serialize( const QList< QGraphicsItem* > &items, QDataStream &ds ) {
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      ds << item->type( );
      ds << static_cast< quint64 >( elm->elementType( ) );
      elm->save( ds );
    }
  }
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == QNEConnection::Type ) {
      QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
      ds << item->type( );
      conn->save( ds );
    }
  }
}

QList< QGraphicsItem* > SerializationFunctions::deserialize( Editor *editor,
                                                             QDataStream &ds,
                                                             double version,
                                                             QString parentFile,
                                                             QMap< quint64, QNEPort* > portMap ) {
  QList< QGraphicsItem* > itemList;
  while( !ds.atEnd( ) ) {
    int type;
    ds >> type;
    if( type == GraphicElement::Type ) {
      quint64 elmType;
      ds >> elmType;
      GraphicElement *elm = ElementFactory::buildElement( ( ElementType ) elmType, editor );
      if( elm ) {
        itemList.append( elm );
        elm->load( ds, portMap, version );
        if( elm->elementType( ) == ElementType::BOX ) {
          Box *box = qgraphicsitem_cast< Box* >( elm );
//          box->setParentFile( parentFile ); // TODO Parent file handling
          editor->loadBox( box, box->getFile( ) );
        }
        elm->setSelected( true );
      }
      else {
        throw( std::runtime_error( "Could not build element." ) );
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
      throw( std::runtime_error( "Invalid element type. Data is possibly corrupted." ) );
    }
  }
  return( itemList );
}
#include <iostream>

QList< QGraphicsItem* > SerializationFunctions::load( Editor *editor, QDataStream &ds, QString parentFile,
                                                      Scene *scene ) {
  QString str;
  ds >> str;
  if( !str.startsWith( QApplication::applicationName( ) ) ) {
    throw( std::runtime_error( "Invalid file format." ) );
  }
  double version = str.split( " " ).at( 1 ).toDouble( );

  QRectF rect;
  if( version >= 1.4 ) {
    ds >> rect;
  }
  QList< QGraphicsItem* > items = deserialize( editor, ds, version, parentFile );
  if( scene ) {
    foreach( QGraphicsItem * item, items ) {
      scene->addItem( item );
    }
    scene->setSceneRect( scene->itemsBoundingRect( ) );
    if( !scene->views( ).empty( ) ) {
      QGraphicsView *view = scene->views( ).first( );
      rect = rect.united( view->rect( ) );
      rect.moveCenter( QPointF( 0, 0 ) );
      scene->setSceneRect( scene->sceneRect( ).united( rect ) );
      view->centerOn( scene->itemsBoundingRect( ).center( ) );
    }
  }
  return( items );
}
