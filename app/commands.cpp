#include "box.h"
#include "commands.h"
#include "editor.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "scene.h"
#include "serializationfunctions.h"

#include <QApplication>
#include <QDebug>
#include <cmath>
#include <stdexcept>

QList< QGraphicsItem* > loadList( const QList< QGraphicsItem* > &aItems, QVector< int > &ids ) {
  QList< QGraphicsItem* > items;
  for( QGraphicsItem *item : aItems ) {
    if( item->type( ) == GraphicElement::Type ) {
      items.append( item );
    }
  }
  for( QGraphicsItem *item : aItems ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      QVector< QNEPort* > portsList = elm->inputs( );
      portsList.append( elm->outputs( ) );
      for( QNEPort *port : portsList ) {
        for( QNEConnection *conn : port->connections( ) ) {
          if( !items.contains( conn ) ) {
            items.append( conn );
          }
        }
      }
    }
  }
  for( QGraphicsItem *item : aItems ) {
    if( item->type( ) == QNEConnection::Type ) {
      if( !items.contains( item ) ) {
        items.append( item );
      }
    }
  }
  ids.reserve( items.size( ) );
  for( QGraphicsItem *item : items ) {
    ItemWithId *iwid = dynamic_cast< ItemWithId* >( item );
    if( iwid ) {
      ids.append( iwid->id( ) );
    }
  }
  return items;
}


QList< QGraphicsItem* > findItems( const QVector< int > &ids, Editor *editor ) {
  QList< QGraphicsItem* > items;
  items.reserve( ids.size( ) );
  for( QGraphicsItem *item : editor->getScene( )->items( ) ) {
    ItemWithId *iwid = dynamic_cast< ItemWithId* >( item );
    if( iwid && ids.contains( iwid->id( ) ) ) {
      items.append( item );
    }
  }
  if( items.size( ) != ids.size( ) ) {
    throw std::runtime_error( "One or more elements was not found on scene." );
  }
  return( items );
}

void saveitems( QByteArray &itemData, const QList< QGraphicsItem* > &items ) {
/*  qDebug( ) << "Saving Items"; */
  itemData.clear( );
  /* We will store the elements before the connections. */
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  SerializationFunctions::serialize( items, dataStream );
}

void addItems( Editor *editor, QList< QGraphicsItem* > items ) {
  for( QGraphicsItem *item : items ) {
    editor->getScene( )->addItem( item );
    item->setSelected( true );
  }
}

void loadItems( QByteArray &itemData, const QVector<int> &ids, Editor *editor ) {
  if( itemData.isEmpty( ) ) {
    return;
  }
/*  qDebug( ) << "Loading Items"; */
/*
 * Assuming that all connections are stored after the elements, we will deserialize the elements first.
 * We will store one additional information: The element IDs!
 */
  QDataStream dataStream( &itemData, QIODevice::ReadOnly );
  double version = QApplication::applicationVersion( ).toDouble( );
  QList< QGraphicsItem* > items =
    SerializationFunctions::deserialize( editor,
                                         dataStream,
                                         version,
                                         GlobalProperties::currentFile );
  if( items.size( ) != ids.size( ) ) {
    throw std::runtime_error( "One or more elements was not found on scene." );
  }
  for( int i = 0; i < items.size( ); ++i ) {
    ItemWithId *iwid = dynamic_cast< ItemWithId* >( items[ i ] );
    if( iwid ) {
      iwid->setId( ids[ i ] );
    }
  }
  addItems( editor, items );
}

void deleteItems( const QList< QGraphicsItem* > &items, Editor *editor ) {
  for( QGraphicsItem *item : items ) {
    editor->getScene( )->removeItem( item );
    delete item;
  }
}


AddItemsCommand::AddItemsCommand( GraphicElement *aItem, Editor *aEditor, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  QList< QGraphicsItem* > items;
  items.append( aItem );
  items = loadList( items, ids );
  editor = aEditor;
  addItems( editor, items );
  setText( tr( "Add %1 element" ).arg( aItem->objectName( ) ) );
}

AddItemsCommand::AddItemsCommand( QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  QList< QGraphicsItem* > items;
  items.append( aItem );
  items = loadList( items, ids );
  editor = aEditor;
  addItems( editor, items );
  setText( tr( "Add connection" ) );
}

AddItemsCommand::AddItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor,
                                  QUndoCommand *parent ) : QUndoCommand( parent ) {
  QList< QGraphicsItem* > items = loadList( aItems, ids );
  editor = aEditor;
  addItems( editor, items );
  setText( tr( "Add %1 elements" ).arg( items.size( ) ) );
}

DeleteItemsCommand::DeleteItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor,
                                        QUndoCommand *parent )  : QUndoCommand( parent ) {
  QList< QGraphicsItem* > items = loadList( aItems, ids );
  editor = aEditor;
  setText( tr( "Delete %1 elements" ).arg( items.size( ) ) );
}

void AddItemsCommand::undo( ) {
  qDebug( ) << "UNDO " << text( );
  QList< QGraphicsItem* > items
    = findItems( ids, editor );
  saveitems( itemData, items );
  deleteItems( items, editor );
}

void DeleteItemsCommand::undo( ) {
  qDebug( ) << "UNDO " << text( );
  loadItems( itemData, ids, editor );
}

void AddItemsCommand::redo( ) {
  qDebug( ) << "REDO " << text( );
  loadItems( itemData, ids, editor );
}
void DeleteItemsCommand::redo( ) {
  qDebug( ) << "REDO " << text( );
  QList< QGraphicsItem* > items
    = findItems( ids, editor );
  saveitems( itemData, items );
  deleteItems( items, editor );
}


RotateCommand::RotateCommand( const QList< GraphicElement* > &aItems, int aAngle, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  list = aItems;
  angle = aAngle;
  setText( tr( "Rotate %1 degrees" ).arg( angle ) );
  for( GraphicElement *item : list ) {
    positions.append( item->pos( ) );
    item->setPos( item->pos( ) );
  }
}

void RotateCommand::undo( ) {
  QGraphicsScene *scn = list[ 0 ]->scene( );
  scn->clearSelection( );
  for( int i = 0; i < list.size( ); ++i ) {
    GraphicElement *elm = list[ i ];
    if( elm->rotatable( ) ) {
      elm->setRotation( elm->rotation( ) - angle );
    }
    elm->setPos( positions[ i ] );
    elm->update( );
    elm->setSelected( true );
  }
}

void RotateCommand::redo( ) {
  QGraphicsScene *scn = list[ 0 ]->scene( );
  scn->clearSelection( );
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
    elm->setSelected( true );
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
  setText( tr( "Rotate %1 degrees" ).arg( angle ) );
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
  setText( tr( "Move elements" ) );
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
  setText( tr( "Update %1 elements" ).arg( elements.size( ) ) );

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
  c2 = ElementFactory::instance( )->buildConnection( );
  setText( tr( "Wire split" ) );
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
                            QUndoCommand *parent ) : QUndoCommand( parent ) {
  old_elements = elements;
  scene = editor->getScene( );
  for( GraphicElement *oldElm : elements ) {
    GraphicElement *newElm = ElementFactory::buildElement( type, editor );
    newElm->setInputSize( oldElm->inputSize( ) );
    new_elements.append( newElm );
    newElm->setPos( oldElm->pos( ) );
    if( newElm->rotatable( ) && oldElm->rotatable( ) ) {
      newElm->setRotation( oldElm->rotation( ) );
    }
    if( ( newElm->elementType( ) == ElementType::NOT ) && ( oldElm->elementType( ) == ElementType::NODE ) ) {
      newElm->setRotation( oldElm->rotation( ) + 90.0 );
    }
    else if( ( newElm->elementType( ) == ElementType::NODE ) && ( oldElm->elementType( ) == ElementType::NOT ) ) {
      newElm->setRotation( oldElm->rotation( ) - 90.0 );
    }
    if( newElm->hasLabel( ) && oldElm->hasLabel( ) ) {
      newElm->setLabel( oldElm->getLabel( ) );
    }
    if( newElm->hasColors( ) && oldElm->hasColors( ) ) {
      newElm->setColor( oldElm->getColor( ) );
    }
    if( newElm->hasFrequency( ) && oldElm->hasFrequency( ) ) {
      newElm->setFrequency( oldElm->getFrequency( ) );
    }
    if( newElm->hasTrigger( ) && oldElm->hasTrigger( ) ) {
      newElm->setTrigger( oldElm->getTrigger( ) );
    }
  }
  setText( tr( "Morph %1 elements to %2" ).arg( elements.size( ) ).arg( new_elements.front( )->objectName( ) ) );
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
      while( !oldElm->input( in )->connections( ).isEmpty( ) ) {
        QNEConnection *conn = oldElm->input( in )->connections( ).first( );
        if( conn->port1( ) == oldElm->input( in ) ) {
          conn->setPort1( newElm->input( in ) );
        }
        else if( conn->port2( ) == oldElm->input( in ) ) {
          conn->setPort2( newElm->input( in ) );
        }
      }
    }
    for( int out = 0; out < oldElm->outputSize( ); ++out ) {
      while( !oldElm->output( out )->connections( ).isEmpty( ) ) {
        QNEConnection *conn = oldElm->output( out )->connections( ).first( );
        if( conn->port1( ) == oldElm->output( out ) ) {
          conn->setPort1( newElm->output( out ) );
        }
        else if( conn->port2( ) == oldElm->output( out ) ) {
          conn->setPort2( newElm->output( out ) );
        }
      }
    }
    scene->removeItem( oldElm );
    scene->addItem( newElm );
    newElm->updatePorts( );
  }
}


ChangeInputSZCommand::ChangeInputSZCommand( const QVector< GraphicElement* > &elements,
                                            int newInputSize,
                                            QUndoCommand *parent ) : QUndoCommand( parent ) {
  m_elements = elements;
  m_newInputSize = newInputSize;
  if( !elements.isEmpty( ) ) {
    scene = elements.front( )->scene( );
  }
  setText( tr( "Change input size to %1" ).arg( newInputSize ) );
}

void ChangeInputSZCommand::redo( ) {
  if( !m_elements.isEmpty( ) && m_elements.front( )->scene( ) ) {
    scene->clearSelection( );
  }
  serializationOrder.clear( );
  m_oldData.clear( );
  QDataStream dataStream( &m_oldData, QIODevice::WriteOnly );
  for( int i = 0; i < m_elements.size( ); ++i ) {
    GraphicElement *elm = m_elements[ i ];
    elm->save( dataStream );
    serializationOrder.append( elm );
    for( int in = m_newInputSize; in < elm->inputSize( ); ++in ) {
      for( QNEConnection *conn : elm->input( in )->connections( ) ) {
        QNEPort *otherPort = conn->otherPort( elm->input( in ) );
        otherPort->graphicElement( )->save( dataStream );
        serializationOrder.append( otherPort->graphicElement( ) );
      }
    }
  }
  for( int i = 0; i < m_elements.size( ); ++i ) {
    GraphicElement *elm = m_elements[ i ];
    for( int in = m_newInputSize; in < elm->inputSize( ); ++in ) {
      while( !elm->input( in )->connections( ).isEmpty( ) ) {
        QNEConnection *conn = elm->input( in )->connections( ).front( );
        conn->save( dataStream );
        scene->removeItem( conn );
        QNEPort *otherPort = conn->otherPort( elm->input( in ) );
        elm->input( in )->disconnect( conn );
        otherPort->disconnect( conn );
      }
    }
    elm->setInputSize( m_newInputSize );
    elm->setSelected( true );
  }
}


void ChangeInputSZCommand::undo( ) {
  if( !m_elements.isEmpty( ) && m_elements.front( )->scene( ) ) {
    scene->clearSelection( );
  }
  QDataStream dataStream( &m_oldData, QIODevice::ReadOnly );
  double version = QApplication::applicationVersion( ).toDouble( );
  QMap< quint64, QNEPort* > portMap;
  for( GraphicElement *elm : serializationOrder ) {
    elm->load( dataStream, portMap, version );
  }
  for( int i = 0; i < m_elements.size( ); ++i ) {
    GraphicElement *elm = m_elements[ i ];
    for( int in = m_newInputSize; in < elm->inputSize( ); ++in ) {
      QNEConnection *conn = ElementFactory::buildConnection();
      conn->load( dataStream, portMap );
      scene->addItem( conn );
    }
    elm->setSelected( true );
  }
}
