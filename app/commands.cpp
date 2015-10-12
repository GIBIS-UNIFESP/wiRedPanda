#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"
#include "scene.h"
#include "serializationfunctions.h"

#include <QDebug>

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
    if( ( item->type( ) == QNEConnection::Type ) && !item->scene( ) ) {
      items.removeAll( item );
      delete item;
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


RotateCommand::RotateCommand( const QList< GraphicElement* > &aItems, int aAngle, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  list = aItems;
  angle = aAngle;
  setText( QString( "Rotate %1 degrees" ).arg( angle ) );
  foreach( GraphicElement * item, list ) {
    positions.append( item->pos( ) );
    item->setPos( item->pos( ) );
  }
}

void RotateCommand::undo( ) {
  for( int i = 0; i < list.size( ); ++i ) {
    GraphicElement *elm = list[ i ];
    if( elm->rotatable( ) ) {
      elm->setRotation( elm->rotation( ) - angle );
    }
    elm->setPos( positions[ i ] );
    elm->update( );
  }
}

void RotateCommand::redo( ) {
  double cx = 0, cy = 0;
  int sz = 0;
  foreach( GraphicElement * item, list ) {
    cx += item->pos( ).x( );
    cy += item->pos( ).y( );
    sz++;
  }
  cx /= sz;
  cy /= sz;
  foreach( GraphicElement * elm, list ) {
    QTransform transform;
    transform.translate( cx, cy );
    transform.rotate( angle );
    transform.translate( -cx, -cy );
    if( elm->rotatable( ) ) {
      elm->setRotation( elm->rotation( ) + angle );
    }
    elm->setPos( transform.map( elm->pos( ) ) );
    elm->update( );
  }
}

bool RotateCommand::mergeWith( const QUndoCommand *command ) {
  const RotateCommand *rotateCommand = static_cast< const RotateCommand* >( command );
  if( !rotateCommand ) {
    return( false );
  }
  angle = ( angle + rotateCommand->angle ) % 360;
  setText( QString( "Rotate  AA %1 degrees" ).arg( angle ) );
  undo( );
  redo( );
  return( true );
}


int RotateCommand::id( ) const {
  return( Id );
}


MoveCommand::MoveCommand( const QList< GraphicElement* > &list,
                          const QList< QPointF > &aOldPositions,
                          QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  myList = list;
  oldPositions = aOldPositions;
  foreach( GraphicElement * elm, list ) {
    newPositions.append( elm->pos( ) );
  }
  undo( );
  setText( QString( "Move %1 elements" ).arg( list.size( ) ) );
}

void MoveCommand::undo( ) {
  for( int i = 0; i < myList.size( ); ++i ) {
    myList[ i ]->setPos( oldPositions[ i ] );
  }
}

void MoveCommand::redo( ) {
  for( int i = 0; i < myList.size( ); ++i ) {
    myList[ i ]->setPos( newPositions[ i ] );
  }
}
