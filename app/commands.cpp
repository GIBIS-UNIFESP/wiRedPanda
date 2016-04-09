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
      if( !items.contains( item ) ) {
        items.append( item );
      }
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
  return( items );
}


QList< QGraphicsItem* > findItems( const QVector< int > &ids ) {
  QList< QGraphicsItem* > items;
  for( int id : ids ) {
    QGraphicsItem *item = dynamic_cast< QGraphicsItem* >( ElementFactory::getItemById( id ) );
    if( item ) {
      items.append( item );
    }
  }
  if( items.size( ) != ids.size( ) ) {
    throw std::runtime_error( "One or more items was not found on the scene." );
  }
  return( items );
}

QList< GraphicElement* > findElements( const QVector< int > &ids ) {
  QList< GraphicElement* > items;
  for( int id : ids ) {
    GraphicElement *item = dynamic_cast< GraphicElement* >( ElementFactory::getItemById( id ) );
    if( item ) {
      items.append( item );
    }
  }
  if( items.size( ) != ids.size( ) ) {
    throw std::runtime_error( "One or more elements was not found on the scene." );
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

void loadItems( QByteArray &itemData, const QVector< int > &ids, Editor *editor ) {
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
      ElementFactory::updateItemId( iwid, ids[ i ] );
    }
  }
  addItems( editor, items );
}

void deleteItems( const QList< QGraphicsItem* > &items, Editor *editor ) {
  QVector< QGraphicsItem* > itemsVec = items.toVector( );
  /* Delete items on reverse order */
  for( int i = itemsVec.size( ) - 1; i >= 0; --i ) {
    editor->getScene( )->removeItem( itemsVec[ i ] );
    delete itemsVec[ i ];
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
                                        QUndoCommand *parent ) : QUndoCommand( parent ) {
  QList< QGraphicsItem* > items = loadList( aItems, ids );
  editor = aEditor;
  setText( tr( "Delete %1 elements" ).arg( items.size( ) ) );
}

void AddItemsCommand::undo( ) {
  qDebug( ) << "UNDO " << text( );
  QList< QGraphicsItem* > items
    = findItems( ids );
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
    = findItems( ids );
  saveitems( itemData, items );
  deleteItems( items, editor );
}


RotateCommand::RotateCommand( const QList< GraphicElement* > &aItems, int aAngle, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  angle = aAngle;
  setText( tr( "Rotate %1 degrees" ).arg( angle ) );
  ids.reserve( aItems.size( ) );
  positions.reserve( aItems.size( ) );
  for( GraphicElement *item : aItems ) {
    positions.append( item->pos( ) );
    item->setPos( item->pos( ) );
    ids.append( item->id( ) );
  }
}

void RotateCommand::undo( ) {
  QList< GraphicElement* > list = findElements( ids );
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
  QList< GraphicElement* > list = findElements( ids );
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
  if( ids.size( ) != rotateCommand->ids.size( ) ) {
    return( false );
  }
  QVector< GraphicElement* > list = findElements( ids ).toVector( );
  QVector< GraphicElement* > list2 = findElements( rotateCommand->ids ).toVector( );
  for( int i = 0; i < list.size( ); ++i ) {
    if( list[ i ] != list2[ i ] ) {
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
  oldPositions = aOldPositions;
  newPositions.reserve( list.size( ) );
  ids.reserve( list.size( ) );
  for( GraphicElement *elm : list ) {
    ids.append( elm->id( ) );
    newPositions.append( elm->pos( ) );
  }
  setText( tr( "Move elements" ) );
}

void MoveCommand::undo( ) {
  QVector< GraphicElement* > elms = findElements( ids ).toVector( );
  for( int i = 0; i < elms.size( ); ++i ) {
    elms[ i ]->setPos( oldPositions[ i ] );
  }
}

void MoveCommand::redo( ) {
  QVector< GraphicElement* > elms = findElements( ids ).toVector( );
  for( int i = 0; i < elms.size( ); ++i ) {
    elms[ i ]->setPos( newPositions[ i ] );
  }
}


UpdateCommand::UpdateCommand( const QVector< GraphicElement* > &elements, QByteArray oldData,
                              QUndoCommand *parent ) : QUndoCommand( parent ) {
  m_oldData = oldData;
  ids.reserve( elements.size( ) );
  QDataStream dataStream( &m_newData, QIODevice::WriteOnly );
  for( GraphicElement *elm : elements ) {
    elm->save( dataStream );
    ids.append( elm->id( ) );
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
  QVector< GraphicElement* > elements = findElements( ids ).toVector( );
  QDataStream dataStream( &itemData, QIODevice::ReadOnly );
  QMap< quint64, QNEPort* > portMap;
  if( !elements.isEmpty( ) && elements.front( )->scene( ) ) {
    elements.front( )->scene( )->clearSelection( );
  }
  double version = QApplication::applicationVersion( ).toDouble( );
  if( !elements.isEmpty( ) ) {
    for( GraphicElement *elm : elements ) {
      elm->load( dataStream, portMap, version );
      elm->setSelected( true );
    }
  }
}


SplitCommand::SplitCommand( QNEConnection *conn, QPointF point, Editor *aEditor, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  Scene *customScene = aEditor->getScene( );
  GraphicElement * node = ElementFactory::buildElement( ElementType::NODE, aEditor );
  QNEConnection *conn2 = ElementFactory::instance( )->buildConnection( );

  /* Align node to Grid */
  nodePos = point - QPointF( node->boundingRect( ).center( ) );
  if( customScene ) {
    int gridSize = customScene->gridSize( );
    qreal xV = qRound( nodePos.x( ) / gridSize ) * gridSize;
    qreal yV = qRound( nodePos.y( ) / gridSize ) * gridSize;
    nodePos = QPointF( xV, yV );
  }

  /* Rotate line according to angle between p1 and p2 */
  nodeAngle = conn->angle( );
  nodeAngle = 360 - 90 * ( std::round( nodeAngle / 90.0 ) );

  /* Assingning class attributes */
  editor = aEditor;

  elm1_id = conn->port1( )->graphicElement( )->id( );
  elm2_id = conn->port2( )->graphicElement( )->id( );

  c1_id = conn->id( );
  c2_id = conn2->id( );

  node_id = node->id( );

  setText( tr( "Wire split" ) );
}

QNEConnection* findConn( int id ) {
  return( dynamic_cast< QNEConnection* >( ElementFactory::getItemById( id ) ) );
}

GraphicElement* findElm( int id ) {
  return( dynamic_cast< GraphicElement* >( ElementFactory::getItemById( id ) ) );
}

void SplitCommand::redo( ) {
  QNEConnection *c1 = findConn( c1_id );
  QNEConnection *c2 = findConn( c2_id );
  GraphicElement *node = findElm( node_id );
  GraphicElement *elm1 = findElm( elm1_id );
  GraphicElement *elm2 = findElm( elm2_id );
  if( !c2 ) {
    c2 = ElementFactory::buildConnection( );
    ElementFactory::updateItemId( c2, c2_id );
  }
  if( !node ) {
    node = ElementFactory::buildElement( ElementType::NODE, editor );
    ElementFactory::updateItemId( node, node_id );
  }
  if( c1 && c2 && elm1 && elm2 && node ) {
    node->setPos( nodePos );
    node->setRotation( nodeAngle );

    c2->setPort1( node->output( ) );
    c2->setPort2( c1->port2( ) );
    c1->setPort2( node->input( ) );


    editor->getScene( )->addItem( node );
    editor->getScene( )->addItem( c2 );

    c1->updatePosFromPorts( );
    c1->updatePath( );
    c2->updatePosFromPorts( );
    c2->updatePath( );
  }
  else {
    throw std::runtime_error( QString( "Error tryng to undo %1" ).arg( text( ) ).toStdString( ) );
  }
}

void SplitCommand::undo( ) {
  QNEConnection *c1 = findConn( c1_id );
  QNEConnection *c2 = findConn( c2_id );
  GraphicElement *node = findElm( node_id );
  GraphicElement *elm1 = findElm( elm1_id );
  GraphicElement *elm2 = findElm( elm2_id );
  if( c1 && c2 && elm1 && elm2 && node ) {
    c1->setPort2(c2->port2());

    c1->updatePosFromPorts( );
    c1->updatePath( );

    editor->getScene( )->removeItem( c2 );
    editor->getScene( )->removeItem( node );

    delete c2;
    delete node;
  }
  else {
    throw std::runtime_error( QString( "Error tryng to undo %1" ).arg( text( ) ).toStdString( ) );
  }
}

MorphCommand::MorphCommand( const QVector< GraphicElement* > &elements,
                            ElementType aType,
                            Editor *aEditor,
                            QUndoCommand *parent ) : QUndoCommand( parent ) {
  newtype = aType;
  editor = aEditor;
  ids.reserve(elements.size());
  types.reserve(elements.size());
  for( GraphicElement *oldElm : elements ) {
    ids.append(oldElm->id());
    types.append(oldElm->elementType());
  }
  setText( tr( "Morph %1 elements to %2" ).arg( elements.size( ) ).arg( elements.front( )->objectName( ) ) );
}

void MorphCommand::undo( ) {
  QVector <GraphicElement *> newElms = findElements(ids).toVector();
  QVector <GraphicElement *> oldElms( newElms.size() );
  for( int i = 0; i < ids.size(); ++i ){
    oldElms[i] = ElementFactory::buildElement(types[i], editor);
  }
  transferConnections( newElms, oldElms );
}

void MorphCommand::redo( ) {
  QVector <GraphicElement *> oldElms = findElements(ids).toVector();
  QVector <GraphicElement *> newElms( oldElms.size() );
  for( int i = 0; i < ids.size(); ++i ){
    newElms[i] = ElementFactory::buildElement(newtype, editor);
  }
  transferConnections( oldElms, newElms );
}

void MorphCommand::transferConnections( QVector< GraphicElement* > from, QVector< GraphicElement* > to ) {
  for( int elm = 0; elm < from.size( ); ++elm ) {
    GraphicElement *oldElm = from[ elm ];
    GraphicElement *newElm = to[ elm ];
    newElm->setInputSize( oldElm->inputSize( ) );

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

    int oldId = oldElm->id();
    editor->getScene()->removeItem( oldElm );
    delete oldElm;

    ElementFactory::updateItemId(newElm, oldId);
    editor->getScene()->addItem( newElm );
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
      QNEConnection *conn = ElementFactory::buildConnection( );
      conn->load( dataStream, portMap );
      scene->addItem( conn );
    }
    elm->setSelected( true );
  }
}
