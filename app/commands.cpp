#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"
#include "scene.h"
#include "serializationfunctions.h"

#include <QApplication>
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
  if( list.size( ) != rotateCommand->list.size( ) ) {
    return( false );
  }
  for( int i = 0; i < list.size( ); ++i ) {
    if( list[ i ] != rotateCommand->list[ i ] ) {
      return( false );
    }
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
  setText( QString( "Move elements" ) );
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


UpdateCommand::UpdateCommand( GraphicElement *element, QByteArray oldData,
                              QUndoCommand *parent ) : QUndoCommand( parent ) {
  m_element = element;
  m_oldData = oldData;
  QDataStream dataStream( &m_newData, QIODevice::WriteOnly );
  m_element->save( dataStream );
  setText( QString( "Update %1 element" ).arg( element->objectName( ) ) );

}

void UpdateCommand::undo( ) {
  loadData( m_oldData );
}

void UpdateCommand::redo( ) {
  loadData( m_newData );
}

void UpdateCommand::loadData( QByteArray itemData ) {
  QDataStream dataStream( &itemData, QIODevice::ReadOnly );
  QMap< quint64, QNEPort* > portMap;
  double version = QApplication::applicationVersion( ).toDouble( );
  m_element->load( dataStream, portMap, version );
}


SplitCommand::SplitCommand( QNEConnection *conn, QPointF point, QUndoCommand *parent ) : QUndoCommand( parent ) {
  node = new Node( );
  node->setPos( point - QPointF( node->boundingRect( ).center( ) ) );
  p1 = conn->port1( );
  p2 = conn->port2( );
  trueP1 = p1;
  trueP2 = p2;
  if( p2->isOutput( ) ) {
    std::swap( p1, p2 );
  }
  c1 = conn;
  c2 = new QNEConnection( );
  setText( QString( "Wire split" ) );

}

SplitCommand::~SplitCommand( ) {
  if( !c2->scene( ) ) {
    delete c2;
  }
  if( !node->scene( ) ) {
    delete node;
  }
}

void SplitCommand::undo( ) {

  node->inputs( ).front( )->disconnect( c1 );
  node->outputs( ).front( )->disconnect( c2 );
  c1->scene( )->removeItem( node );

  p2->disconnect( c2 );
  c1->scene( )->removeItem( c2 );

  c1->setPort1( trueP1 );
  c1->setPort2( trueP2 );

  c1->updatePosFromPorts( );
  c1->updatePath( );
}

void SplitCommand::redo( ) {
  p1->disconnect( c1 );
  p2->disconnect( c1 );
  c1->setPort1( p1 );
  c1->setPort2( node->inputs( ).front( ) );

  c2->setPort1( node->outputs( ).front( ) );
  c2->setPort2( p2 );

  c1->scene( )->addItem( node );
  c1->scene( )->addItem( c2 );

  c1->updatePosFromPorts( );
  c1->updatePath( );

  c2->updatePosFromPorts( );
  c2->updatePath( );
}
