#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"
#include "scene.h"
#include "serializationfunctions.h"

/* TODO Criar comandos usando mesma técnica do copy n' paste. */

AddElementCommand::AddElementCommand( GraphicElement *aItem, Scene *aScene, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  item = aItem;
  scene = aScene;
  setText( QString( "Add %1 element" ).arg( item->objectName( ) ) );
}

AddElementCommand::~AddElementCommand( ) {
  if( qgraphicsitem_cast<GraphicElement * >(item) ) {
    delete item;
  }
}

void AddElementCommand::undo( ) {
  scene->removeItem( item );
}

void AddElementCommand::redo( ) {
  scene->addItem( item );
  scene->clearSelection( );
  scene->update( );
}

DeleteElementsCommand::DeleteElementsCommand( const QList< QGraphicsItem* > &aItems,
                                              Editor *aEditor,
                                              QUndoCommand *parent ) : QUndoCommand( parent ) {
  editor = aEditor;
  items = aItems;
  setText( QString( "Delete %1 elements" ).arg( items.size( ) ) );
}

DeleteElementsCommand::~DeleteElementsCommand( ) {
  foreach( QGraphicsItem * item, items ) {
    if( qgraphicsitem_cast<GraphicElement * >(item) ) {
      delete item;
    }
  }
}

void DeleteElementsCommand::undo( ) {
  QDataStream storedData( &itemData, QIODevice::ReadOnly );
  Scene *scene = editor->getScene( );
  foreach( QGraphicsItem * item, items ) {
    scene->addItem( item );
  }
  while( !storedData.atEnd( ) ) {
    QNEConnection *conn = new QNEConnection( 0 );
    scene->addItem( conn );
    conn->load( storedData );
  }
  itemData.clear( );
}

void DeleteElementsCommand::redo( ) {
  itemData.clear( );
  QDataStream storedData( &itemData, QIODevice::WriteOnly );
  Scene *scene = editor->getScene( );
  QList< QGraphicsItem* > savedItems;
  QList< QNEConnection* > connections;
  foreach( QGraphicsItem * item, items ) {
    scene->removeItem( item );
    if( item->type( ) == GraphicElement::Type ) {
      savedItems.append( item );
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      QVector< QNEPort* > ports = elm->inputs( );
      ports.append( elm->outputs( ) );
      foreach( QNEPort * port, ports ) {
        foreach( QNEConnection * conn, port->connections( ) ) {
          if( !connections.contains( conn ) ) {
            connections.append( conn );
          }
        }
      }
    }
    else if( item->type( ) == QNEConnection::Type ) {
      QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
      if( !connections.contains( conn ) ) {
        connections.append( conn );
      }
    }
  }
  foreach( QNEConnection * conn, connections ) {
    conn->save( storedData );
    delete conn;
  }
  items.swap( savedItems );
}
