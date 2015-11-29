#include "box.h"
#include "editor.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "serializationfunctions.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QMessageBox>

void SerializationFunctions::serialize( const QList< QGraphicsItem* > &items, QDataStream &ds ) {
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == GraphicElement::Type ) {
      ds << item->type( );
      ds << ( quint64 ) ( ( GraphicElement* ) item )->elementType( );
      ( ( GraphicElement* ) item )->save( ds );
    }
  }
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == QNEConnection::Type ) {
      ds << item->type( );
      ( ( QNEConnection* ) item )->save( ds );
    }
  }
}

QList< QGraphicsItem* > SerializationFunctions::deserialize( Editor *editor, QDataStream &ds, double version ) {
  QList< QGraphicsItem* > itemList;
  QMap< quint64, QNEPort* > portMap;
  while( !ds.atEnd( ) ) {
    int type;
    ds >> type;
    if( type == GraphicElement::Type ) {
      quint64 elmType;
      ds >> elmType;
      GraphicElement *elm = editor->getFactory( ).buildElement( ( ElementType ) elmType, editor );
      if( elm ) {
#ifdef DEBUG
//        qDebug() << "Loaded " << elm->objectName() << " : " << elm->getLabel();
#endif
        itemList.append( elm );
        elm->load( ds, portMap, version );
        if( elm->elementType( ) == ElementType::BOX ) {
          Box *box = qgraphicsitem_cast< Box* >( elm );
          editor->loadBox( box, box->getFile( ) );
        }
        elm->setSelected( true );
      }
      else {
        throw( std::runtime_error( "Could not build element." ) );
      }
    }
    else if( type == QNEConnection::Type ) {
      QNEConnection *conn = new QNEConnection( 0 );
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

QList< QGraphicsItem* > SerializationFunctions::load( Editor *editor, QDataStream &ds, Scene *scene ) {
  QString str;
  ds >> str;
  qDebug() << "Header: " << str;
  if( !str.startsWith( QApplication::applicationName( ) ) ) {
    throw( std::runtime_error( "Invalid file format." ) );
  }

// else if( !str.endsWith( QApplication::applicationVersion( ) ) ) {
//    QMessageBox::warning( nullptr, "Warning!", "File opened in compatibility mode.", QMessageBox::Ok,
//    QMessageBox::NoButton );
// }

  double version = str.split( " " ).at( 1 ).toDouble( );

  QRectF rect;
  if( version >= 1.4 ) {
    ds >> rect;
  }
  QList< QGraphicsItem* > items = deserialize( editor, ds, version );
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
      view->ensureVisible( scene->itemsBoundingRect( ) );
    }
  }
  return( items );
}
