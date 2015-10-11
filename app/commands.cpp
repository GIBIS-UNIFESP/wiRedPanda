#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"
#include "scene.h"
#include "serializationfunctions.h"

/* TODO Criar comandos usando mesma técnica do copy n' paste. */

AddItemsCommand::AddItemsCommand( GraphicElement *aItem, Editor *aEditor, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  items.append( aItem );
  editor = aEditor;
  setText( QString( "Add %1 element" ).arg( aItem->objectName( ) ) );
}

AddItemsCommand::AddItemsCommand( QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  items.append( aItem );
  editor = aEditor;
  setText( QString( "Add connection" ) );
}

AddItemsCommand::AddItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor,
                                  QUndoCommand *parent ) : QUndoCommand( parent ) {
  items = aItems;
  editor = aEditor;
  setText( QString( "Add %1 elements" ).arg( items.size( ) ) );
}

AddItemsCommand::~AddItemsCommand( ) {
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == QNEConnection::Type ) {
      delete item;
      items.removeAll( item );
    }
  }

  foreach( QGraphicsItem * item, items ) {
    if( !item->scene( ) ) {
      delete item;
    }
  }
}

void AddItemsCommand::undo( ) {
  foreach( QGraphicsItem * item, items ) {
    if( item->scene( ) ) {
      editor->getScene( )->removeItem( item );
    }
  }
}

void AddItemsCommand::redo( ) {
  foreach( QGraphicsItem * item, items ) {
    if( !item->scene( ) ) {
      editor->getScene( )->addItem( item );
    }
    item->setSelected( true );
  }
  editor->getScene( )->clearSelection( );
  editor->getScene( )->update( );
}

DeleteItemsCommand::DeleteItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor,
                                        QUndoCommand *parent ) : QUndoCommand( parent ) {
  editor = aEditor;
  foreach( QGraphicsItem * item, aItems ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      elements.append( elm );
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

  setText( QString( "Delete %1 elements" ).arg( elements.size( ) ) );
}

void DeleteItemsCommand::undo( ) {
  QDataStream storedData( &itemData, QIODevice::ReadOnly );
  foreach( QGraphicsItem * item, elements ) {
    editor->getScene( )->addItem( item );
  }
  foreach( QNEConnection * conn, connections ) {
    editor->getScene( )->addItem( conn );
    conn->load( storedData );
  }
  itemData.clear( );
  editor->getScene( )->update( );
}

void DeleteItemsCommand::redo( ) {
  itemData.clear( );
  QDataStream storedData( &itemData, QIODevice::WriteOnly );
  foreach( QNEConnection * conn, connections ) {
    conn->save( storedData );
    QNEPort *p1 = conn->port1( );
    if( p1 ) {
      p1->disconnect( conn );
    }
    QNEPort *p2 = conn->port2( );
    if( p2 ) {
      p2->disconnect( conn );
    }
    editor->getScene( )->removeItem( conn );
  }

  foreach( GraphicElement * elm, elements ) {
    editor->getScene( )->removeItem( elm );
  }

}
