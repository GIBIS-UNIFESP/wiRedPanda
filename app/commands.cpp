#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"
#include "scene.h"
#include "serializationfunctions.h"

#include <QApplication>
#include <QDebug>
#include <cmath>

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
  for( QGraphicsItem *item : items ) {
    if( ( item->type( ) == QNEConnection::Type ) && !item->scene( ) ) {
      items.removeAll( item );
      delete item;
    }
  }
  for( QGraphicsItem *item : items ) {
    if( !item->scene( ) ) {
      delete item;
    }
  }
}

void AddItemsCommand::undo( ) {
  for( QGraphicsItem *item : items ) {
    if( item->scene( ) ) {
      editor->getScene( )->removeItem( item );
    }
  }
}

void AddItemsCommand::redo( ) {
  for( QGraphicsItem *item : items ) {
    if( !item->scene( ) ) {
      editor->getScene( )->addItem( item );
    }
    item->setSelected( true );
  }
  editor->getScene( )->clearSelection( );
  editor->getScene( )->update( );
  editor->updateVisibility( );
}

DeleteItemsCommand::DeleteItemsCommand( const QList< QGraphicsItem* > &aItems, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  if( !aItems.isEmpty( ) ) {
    scene = aItems.front( )->scene( );
  }
  for( QGraphicsItem *item : aItems ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      elements.append( elm );
      QVector< QNEPort* > ports = elm->inputs( );
      ports.append( elm->outputs( ) );
      for( QNEPort *port : ports ) {
        for( QNEConnection *conn : port->connections( ) ) {
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
  for( QGraphicsItem *item : elements ) {
    scene->addItem( item );
  }
  for( QNEConnection *conn : connections ) {
    scene->addItem( conn );
    conn->load( storedData );
  }
  itemData.clear( );
  scene->update( );
}

void DeleteItemsCommand::redo( ) {
  itemData.clear( );
  QDataStream storedData( &itemData, QIODevice::WriteOnly );
  for( QNEConnection *conn : connections ) {
    conn->save( storedData );
    QNEPort *p1 = conn->port1( );
    if( p1 ) {
      p1->disconnect( conn );
    }
    QNEPort *p2 = conn->port2( );
    if( p2 ) {
      p2->disconnect( conn );
    }
    scene->removeItem( conn );
  }
  for( GraphicElement *elm : elements ) {
    scene->removeItem( elm );
  }
}


RotateCommand::RotateCommand( const QList< GraphicElement* > &aItems, int aAngle, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  list = aItems;
  angle = aAngle;
  setText( QString( "Rotate %1 degrees" ).arg( angle ) );
  for( GraphicElement *item : list ) {
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
  for( GraphicElement *item : list ) {
    cx += item->pos( ).x( );
    cy += item->pos( ).y( );
    sz++;
  }
  cx /= sz;
  cy /= sz;
  for( GraphicElement *elm : list ) {
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
  for( GraphicElement *elm : list ) {
    newPositions.append( elm->pos( ) );
  }
  setText( QString( "Move elements" ) );
}

void MoveCommand::undo( ) {
/*  qDebug() << "UNDO!!!" << myList.size(); */
  for( int i = 0; i < myList.size( ); ++i ) {
    myList[ i ]->setPos( oldPositions[ i ] );
  }
}

void MoveCommand::redo( ) {
/*  qDebug() << "REDO!!!" << myList.size(); */
  for( int i = 0; i < myList.size( ); ++i ) {
    myList[ i ]->setPos( newPositions[ i ] );
  }
}


UpdateCommand::UpdateCommand( const QVector< GraphicElement* > &elements, QByteArray oldData,
                              QUndoCommand *parent ) : QUndoCommand( parent ) {
  m_elements = elements;
  m_oldData = oldData;
  QDataStream dataStream( &m_newData, QIODevice::WriteOnly );
  for( GraphicElement *elm : elements ) {
    elm->save( dataStream );
  }
  setText( QString( "Update %1 elements" ).arg( elements.size( ) ) );

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
  if( !m_elements.isEmpty( ) && m_elements.front( )->scene( ) ) {
    m_elements.front( )->scene( )->clearSelection( );
  }
  double version = QApplication::applicationVersion( ).toDouble( );
  if( !m_elements.isEmpty( ) ) {
    for( GraphicElement *elm : m_elements ) {
      elm->load( dataStream, portMap, version );
      elm->setSelected( true );
    }
  }
}


SplitCommand::SplitCommand( QNEConnection *conn, QPointF point, QUndoCommand *parent ) : QUndoCommand( parent ) {
  node = new Node( );

  QPointF newPos = point - QPointF( node->boundingRect( ).center( ) );
  Scene *customScene = dynamic_cast< Scene* >( conn->scene( ) );
  if( customScene ) {
    int gridSize = customScene->gridSize( );
    qreal xV = qRound( newPos.x( ) / gridSize ) * gridSize;
    qreal yV = qRound( newPos.y( ) / gridSize ) * gridSize;
    newPos = QPointF( xV, yV );
  }
  node->setPos( newPos );
  int angle = conn->angle( );
  angle = 360 - 90 * ( std::round( angle / 90.0 ) );
  node->setRotation( angle );
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

MorphCommand::MorphCommand( const QVector< GraphicElement* > &elements,
                            ElementType type,
                            Editor *editor,
                            QUndoCommand *parent )  : QUndoCommand(parent){
  old_elements = elements;
  for( GraphicElement *elm : elements ) {
    GraphicElement *newElm = editor->getFactory( ).buildElement( type, editor );
    newElm->setInputSize( elm->inputSize( ) );
    new_elements.append( newElm );
    newElm->setPos(elm->pos());
    if(newElm->rotatable() && elm->rotatable() ){
      newElm->setRotation(elm->rotation());
    }
    editor->getScene()->addItem(newElm);
  }
}

void MorphCommand::undo( ) {
  transferConnections( new_elements, old_elements );
}

void MorphCommand::redo( ) {
  transferConnections( old_elements, new_elements );
}

void MorphCommand::transferConnections( QVector< GraphicElement* > from, QVector< GraphicElement* > to ) {
  for( int elm = 0; elm < from.size( ); ++elm ) {
    GraphicElement *oldElm = from[ elm ];
    GraphicElement *newElm = to[ elm ];
    for( int in = 0; in < oldElm->inputSize( ); ++in ) {
      for( QNEConnection *conn : oldElm->input( in )->connections( ) ) {
        oldElm->input( in )->disconnect( conn );
        if( conn->port1( ) == nullptr ) {
          conn->setPort1( newElm->input( in ) );
        }
        else {
          conn->setPort2( newElm->input( in ) );
        }
      }
    }
    for( int out = 0; out < oldElm->outputSize( ); ++out ) {
      for( QNEConnection *conn : oldElm->output( out )->connections( ) ) {
        oldElm->output( out )->disconnect( conn );
        if( conn->port1( ) == nullptr ) {
          conn->setPort1( newElm->output( out ) );
        }
        else {
          conn->setPort2( newElm->output( out ) );
        }
      }
    }
    oldElm->setVisible( false );
    newElm->setVisible( true );
  }
}
