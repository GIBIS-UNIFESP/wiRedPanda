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

QList< QGraphicsItem* > loadList( const QList< QGraphicsItem* > &aItems, QVector< int > &ids,
                                  QVector< int > &otherIds ) {

  QList< QGraphicsItem* > items;
  /* Stores selected graphicElements */
  for( QGraphicsItem *item : aItems ) {
    if( item->type( ) == GraphicElement::Type ) {
      if( !items.contains( item ) ) {
        items.append( item );
      }
    }
  }
  QList< QGraphicsItem* > connections;
  /* Stores the wires linked to these elements */
  for( QGraphicsItem *item : items ) {
    GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
    QVector< QNEInputPort* > inputsList = elm->inputs( );
    for( QNEInputPort *port : inputsList ) {
      for( QNEConnection *conn : port->connections( ) ) {
        if( !connections.contains( conn ) ) {
          connections.append( conn );
        }
      }
    }
    QVector< QNEOutputPort* > outputsList = elm->outputs( );
    for( QNEOutputPort *port : outputsList ) {
      for( QNEConnection *conn : port->connections( ) ) {
        if( !connections.contains( conn ) ) {
          connections.append( conn );
        }
      }
    }
  }
  /* Stores the other wires selected */
  for( QGraphicsItem *item : aItems ) {
    if( item->type( ) == QNEConnection::Type ) {
      if( !connections.contains( item ) ) {
        connections.append( item );
      }
    }
  }
  items.append( connections );
  ids.reserve( items.size( ) );
  /* Stores the ids of all elements listed in items; */
  for( QGraphicsItem *item : items ) {
    ItemWithId *iwid = dynamic_cast< ItemWithId* >( item );
    if( iwid ) {
      ids.append( iwid->id( ) );
    }
  }
  /* Stores all the elements linked to each connection that will not be deleted. */
  for( QGraphicsItem *item : items ) {
    if( item->type( ) == QNEConnection::Type ) {
      QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
      QNEOutputPort *p1 = conn->start( );
      if( p1 && p1->graphicElement( ) && !ids.contains( p1->graphicElement( )->id( ) ) ) {
        otherIds.append( p1->graphicElement( )->id( ) );
      }
      QNEInputPort *p2 = conn->end( );
      if( p2 && p2->graphicElement( ) && !ids.contains( p2->graphicElement( )->id( ) ) ) {
        otherIds.append( p2->graphicElement( )->id( ) );
      }
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

void saveitems( QByteArray &itemData, const QList< QGraphicsItem* > &items, const QVector< int > &otherIds ) {
  itemData.clear( );
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  QList< GraphicElement* > others = findElements( otherIds );
  for( GraphicElement *elm : others ) {
    elm->save( dataStream );
  }
  SerializationFunctions::serialize( items, dataStream );
}

void addItems( Editor *editor, QList< QGraphicsItem* > items ) {
  editor->getScene( )->clearSelection( );
  for( QGraphicsItem *item : items ) {
    if( item->scene( ) != editor->getScene( ) ) {
      editor->getScene( )->addItem( item );
    }
    if( item->type( ) == GraphicElement::Type ) {
      item->setSelected( true );
    }
  }
}

void loadItems( QByteArray &itemData, const QVector< int > &ids, Editor *editor, QVector< int > &otherIds ) {
  if( itemData.isEmpty( ) ) {
    return;
  }
  QVector< GraphicElement* > otherElms = findElements( otherIds ).toVector( );
  QDataStream dataStream( &itemData, QIODevice::ReadOnly );
  double version = GlobalProperties::version;
  QMap< quint64, QNEPort* > portMap;
  for( GraphicElement *elm : otherElms ) {
    elm->load( dataStream, portMap, version );
  }
  /*
   * Assuming that all connections are stored after the elements, we will deserialize the elements first.
   * We will store one additional information: The element IDs!
   */
  QList< QGraphicsItem* > items =
    SerializationFunctions::deserialize( editor,
                                         dataStream,
                                         version,
                                         GlobalProperties::currentFile,
                                         portMap );
  if( items.size( ) != ids.size( ) ) {
    throw std::runtime_error( QString( "One or more elements was not found on scene. Expected %1, found %2." ).arg( ids
                                                                                                                    .
                                                                                                                    size(
                                                                                                                      ) )
                              .arg(
                                items.size( ) ).toStdString( ) );
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
  items = loadList( items, ids, otherIds );
  editor = aEditor;
  addItems( editor, items );
  setText( tr( "Add %1 element" ).arg( aItem->objectName( ) ) );
}

AddItemsCommand::AddItemsCommand( QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  QList< QGraphicsItem* > items;
  items.append( aItem );
  items = loadList( items, ids, otherIds );
  editor = aEditor;
  addItems( editor, items );
  setText( tr( "Add connection" ) );
}

AddItemsCommand::AddItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor,
                                  QUndoCommand *parent ) : QUndoCommand( parent ) {
  QList< QGraphicsItem* > items = loadList( aItems, ids, otherIds );
  editor = aEditor;
  addItems( editor, items );
  setText( tr( "Add %1 elements" ).arg( items.size( ) ) );
}

DeleteItemsCommand::DeleteItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor,
                                        QUndoCommand *parent ) : QUndoCommand( parent ) {
  QList< QGraphicsItem* > items = loadList( aItems, ids, otherIds );
  editor = aEditor;
  setText( tr( "Delete %1 elements" ).arg( items.size( ) ) );
}

DeleteItemsCommand::DeleteItemsCommand( QGraphicsItem *item, Editor *aEditor, QUndoCommand *parent ) :
  DeleteItemsCommand( QList< QGraphicsItem* >( {
  item
} ), aEditor, parent ) {

}

void AddItemsCommand::undo( ) {
  qDebug( ) << "UNDO " << text( );
  QList< QGraphicsItem* > items = findItems( ids );
  saveitems( itemData, items, otherIds );

  deleteItems( items, editor );
  emit editor->circuitHasChanged( );
}

void DeleteItemsCommand::undo( ) {
  qDebug( ) << "UNDO " << text( );
  loadItems( itemData, ids, editor, otherIds );
  emit editor->circuitHasChanged( );
}

void AddItemsCommand::redo( ) {
  qDebug( ) << "REDO " << text( );
  loadItems( itemData, ids, editor, otherIds );
  emit editor->circuitHasChanged( );
}
void DeleteItemsCommand::redo( ) {
  qDebug( ) << "REDO " << text( );
  QList< QGraphicsItem* > items
    = findItems( ids );
  saveitems( itemData, items, otherIds );

  deleteItems( items, editor );
  emit editor->circuitHasChanged( );
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


UpdateCommand::UpdateCommand( const QVector< GraphicElement* > &elements,
                              QByteArray oldData,
                              Editor *editor,
                              QUndoCommand *parent ) : QUndoCommand( parent ), editor( editor ) {
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
  emit editor->circuitHasChanged( );
}

void UpdateCommand::redo( ) {
  loadData( m_newData );
  emit editor->circuitHasChanged( );
}

void UpdateCommand::loadData( QByteArray itemData ) {
  QVector< GraphicElement* > elements = findElements( ids ).toVector( );
  QDataStream dataStream( &itemData, QIODevice::ReadOnly );
  QMap< quint64, QNEPort* > portMap;
  if( !elements.isEmpty( ) && elements.front( )->scene( ) ) {
    elements.front( )->scene( )->clearSelection( );
  }
  double version = GlobalProperties::version;
  if( !elements.isEmpty( ) ) {
    for( GraphicElement *elm : elements ) {
      elm->load( dataStream, portMap, version );
      elm->setSelected( true );
    }
  }
}


SplitCommand::SplitCommand( QNEConnection *conn, QPointF point, Editor *aEditor, QUndoCommand *parent ) : QUndoCommand(
    parent ) {
  /* TODO Reverse Split command... */
  Scene *customScene = aEditor->getScene( );
  GraphicElement *node = ElementFactory::buildElement( ElementType::NODE, aEditor );
  QNEConnection *conn2 = ElementFactory::instance->buildConnection( );

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

  elm1_id = conn->start( )->graphicElement( )->id( );
  elm2_id = conn->end( )->graphicElement( )->id( );

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

/*    QNEOutputPort *startPort = c1->start( ); */
    QNEInputPort *endPort = c1->end( );
    c2->setStart( node->output( ) );
    c2->setEnd( endPort );
    c1->setEnd( node->input( ) );

    editor->getScene( )->addItem( node );
    editor->getScene( )->addItem( c2 );

    c1->updatePosFromPorts( );
    c1->updatePath( );
    c2->updatePosFromPorts( );
    c2->updatePath( );
  }
  else {
    throw std::runtime_error( QString( "Error tryng to redo %1" ).arg( text( ) ).toStdString( ) );
  }
  emit editor->circuitHasChanged( );
}

void SplitCommand::undo( ) {
  QNEConnection *c1 = findConn( c1_id );
  QNEConnection *c2 = findConn( c2_id );
  GraphicElement *node = findElm( node_id );
  GraphicElement *elm1 = findElm( elm1_id );
  GraphicElement *elm2 = findElm( elm2_id );
  if( c1 && c2 && elm1 && elm2 && node ) {
    c1->setEnd( c2->end( ) );

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
  emit editor->circuitHasChanged( );
}

MorphCommand::MorphCommand( const QVector< GraphicElement* > &elements,
                            ElementType aType,
                            Editor *aEditor,
                            QUndoCommand *parent ) : QUndoCommand( parent ) {
  newtype = aType;
  editor = aEditor;
  ids.reserve( elements.size( ) );
  types.reserve( elements.size( ) );
  for( GraphicElement *oldElm : elements ) {
    ids.append( oldElm->id( ) );
    types.append( oldElm->elementType( ) );
  }
  setText( tr( "Morph %1 elements to %2" ).arg( elements.size( ) ).arg( elements.front( )->objectName( ) ) );
}

void MorphCommand::undo( ) {
  QVector< GraphicElement* > newElms = findElements( ids ).toVector( );
  QVector< GraphicElement* > oldElms( newElms.size( ) );
  for( int i = 0; i < ids.size( ); ++i ) {
    oldElms[ i ] = ElementFactory::buildElement( types[ i ], editor );
  }
  transferConnections( newElms, oldElms );
  emit editor->circuitHasChanged( );
}

void MorphCommand::redo( ) {
  QVector< GraphicElement* > oldElms = findElements( ids ).toVector( );
  QVector< GraphicElement* > newElms( oldElms.size( ) );
  for( int i = 0; i < ids.size( ); ++i ) {
    newElms[ i ] = ElementFactory::buildElement( newtype, editor );
  }
  transferConnections( oldElms, newElms );
  emit editor->circuitHasChanged( );
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
        if( conn->end( ) == oldElm->input( in ) ) {
          conn->setEnd( newElm->input( in ) );
        }
      }
    }
    for( int out = 0; out < oldElm->outputSize( ); ++out ) {
      while( !oldElm->output( out )->connections( ).isEmpty( ) ) {
        QNEConnection *conn = oldElm->output( out )->connections( ).first( );
        if( conn->start( ) == oldElm->output( out ) ) {
          conn->setStart( newElm->output( out ) );
        }
      }
    }
    int oldId = oldElm->id( );
    editor->getScene( )->removeItem( oldElm );
    delete oldElm;

    ElementFactory::updateItemId( newElm, oldId );
    editor->getScene( )->addItem( newElm );
    newElm->updatePorts( );
  }
}


ChangeInputSZCommand::ChangeInputSZCommand( const QVector< GraphicElement* > &elements,
                                            int newInputSize,
                                            Editor *editor,
                                            QUndoCommand *parent ) : QUndoCommand( parent ), editor( editor ) {
  for( GraphicElement *elm : elements ) {
    elms.append( elm->id( ) );
  }
  m_newInputSize = newInputSize;
  if( !elements.isEmpty( ) ) {
    scene = elements.front( )->scene( );
  }
  setText( tr( "Change input size to %1" ).arg( newInputSize ) );
}

void ChangeInputSZCommand::redo( ) {
  const QVector< GraphicElement* > m_elements = findElements( elms ).toVector( );
  if( !m_elements.isEmpty( ) && m_elements.front( )->scene( ) ) {
    scene->clearSelection( );
  }
  QVector< GraphicElement* > serializationOrder;
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
  order.clear( );
  for( GraphicElement *elm : serializationOrder ) {
    order.append( elm->id( ) );
  }
  emit editor->circuitHasChanged( );
}

void ChangeInputSZCommand::undo( ) {
  const QVector< GraphicElement* > m_elements = findElements( elms ).toVector( );
  const QVector< GraphicElement* > serializationOrder = findElements( order ).toVector( );
  if( !m_elements.isEmpty( ) && m_elements.front( )->scene( ) ) {
    scene->clearSelection( );
  }
  QDataStream dataStream( &m_oldData, QIODevice::ReadOnly );
  double version = GlobalProperties::version;
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
  emit editor->circuitHasChanged( );
}

FlipCommand::FlipCommand( const QList< GraphicElement* > &aItems, int aAxis, QUndoCommand *parent ) :
  QUndoCommand( parent ) {
  axis = aAxis;
  setText( tr( "Flip %1 elements in axis %2" ).arg( aItems.size( ) ).arg( aAxis ) );
  ids.reserve( aItems.size( ) );
  positions.reserve( aItems.size( ) );
  double xmin, xmax, ymin, ymax;
  if( aItems.size( ) > 0 ) {
    xmin = xmax = aItems.first( )->pos( ).rx( );
    ymin = ymax = aItems.first( )->pos( ).ry( );
    for( GraphicElement *item : aItems ) {
      positions.append( item->pos( ) );
      item->setPos( item->pos( ) );
      ids.append( item->id( ) );
      xmin = qMin( xmin, item->pos( ).rx( ) );
      xmax = qMax( xmax, item->pos( ).rx( ) );
      ymin = qMin( ymin, item->pos( ).ry( ) );
      ymax = qMax( ymax, item->pos( ).ry( ) );
    }
    minPos = QPointF( xmin, ymin );
    maxPos = QPointF( xmax, ymax );
  }
}

void FlipCommand::undo( ) {
  redo( );
}

void FlipCommand::redo( ) {
  QList< GraphicElement* > list = findElements( ids );
  QGraphicsScene *scn = list[ 0 ]->scene( );
  scn->clearSelection( );
  for( GraphicElement *elm : list ) {
    QPointF pos = elm->pos( );
    if( axis == 0 ) {
      pos.setX( minPos.rx( ) + ( maxPos.rx( ) - pos.rx( ) ) );
    }
    else {
      pos.setY( minPos.ry( ) + ( maxPos.ry( ) - pos.ry( ) ) );
    }
    elm->setPos( pos );
    elm->update( );
    elm->setSelected( true );
  }
}
