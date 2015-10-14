#include "box.h"
#include "commands.h"
#include "editor.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "mainwindow.h"
#include "serializationfunctions.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QtMath>
#include <iostream>
#include <nodes/qneconnection.h>

void Editor::buildSelectionRect( ) {
  selectionRect = new QGraphicsRectItem( );
  selectionRect->setBrush( Qt::NoBrush );
  selectionRect->setFlag( QGraphicsItem::ItemIsSelectable, false );
  selectionRect->setPen( QPen( Qt::darkGray, 1.5, Qt::DotLine ) );
  scene->addItem( selectionRect );
}

Editor::Editor( MainWindow *parent ) : QObject( parent ), scene( nullptr ), editedConn( nullptr ),
  m_hoverPort( nullptr ) {
  mainWindow = parent;
  markingSelectionBox = false;
  undoStack = new QUndoStack( this );
  scene = new Scene( this );
  /* 404552 */
  scene->setBackgroundBrush( QBrush( QColor( "#404552" ) ) );
  scene->setGridSize( 16 );
  install( scene );
  draggingElement = false;
}

Editor::~Editor( ) {
}

void Editor::install( Scene *s ) {
  s->installEventFilter( this );
  simulationController = new SimulationController( s );
  simulationController->start( );
  buildSelectionRect( );
  clear( );
}

void Editor::clear( ) {
  if( scene ) {
    scene->clear( );
  }
  buildSelectionRect( );
  undoStack->clear( );
}

void Editor::deleteElements( ) {
  const QList< QGraphicsItem* > &items = scene->selectedItems( );
  if( !items.isEmpty( ) ) {
    undoStack->push( new DeleteItemsCommand( items, this ) );
  }
}

void Editor::showWires( bool checked ) {
  foreach( QGraphicsItem * c, scene->items( ) ) {
    if( c->type( ) == QNEConnection::Type ) {
      c->setVisible( checked );
    }
  }
}

void Editor::rotate( bool rotateRight ) {
  double angle = 90.0;
  if( !rotateRight ) {
    angle = -angle;
  }
  QList< QGraphicsItem* > list = scene->selectedItems( );
  QList< GraphicElement* > elms;
  foreach( QGraphicsItem * item, list ) {
    GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
    if( elm && ( elm->type( ) == GraphicElement::Type ) ) {
      elms.append( elm );
    }
  }
  if( ( elms.size( ) > 1 ) || ( ( elms.size( ) == 1 ) && elms.front( )->rotatable( ) ) ) {
    undoStack->push( new RotateCommand( elms, angle ) );
  }
}

void Editor::elementUpdated( GraphicElement *element, QByteArray oldData ) {
  undoStack->push( new UpdateCommand( element, oldData ) );
}

QList< QGraphicsItem* > Editor::itemsAt( const QPointF &pos ) {
  return( scene->items( QRectF( pos - QPointF( 7, 7 ), QSize( 13, 13 ) ).normalized( ) ) );
}


QGraphicsItem* Editor::itemAt( const QPointF &pos ) {
  QList< QGraphicsItem* > items = scene->items( pos );
  items.append( itemsAt( pos ) );

  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) == QNEPort::Type ) {
      return( item );
    }
  }
  foreach( QGraphicsItem * item, items ) {
    if( item->type( ) > QGraphicsItem::UserType ) {
      return( item );
    }
  }

  return( nullptr );
}

void Editor::addItem( QGraphicsItem *item ) {
  scene->addItem( item );
}

bool Editor::mousePressEvt( QGraphicsSceneMouseEvent *mouseEvt ) {
  QGraphicsItem *item = itemAt( mousePos );
  if( item && ( item->type( ) == QNEPort::Type ) ) {
    /* When the mouse pressed over an connected input port, the line
     * is disconnected and can be connected in an other port. */
    QNEPort *port2 = ( QNEPort* ) item;
    if( port2->isInput( ) && !port2->connections( ).empty( ) ) {
      /* The last connected line is detached */
      editedConn = port2->connections( ).last( );
      editedConn->setFocus( );

      QNEPort *port1 = editedConn->otherPort( port2 );
      if( port1 ) {
        port2->disconnect( editedConn );
        QList< QGraphicsItem* > itemList;
        itemList.append( editedConn );
        undoStack->push( new DeleteItemsCommand( itemList, this ) );
        editedConn = nullptr;
        editedConn = new QNEConnection( );
        editedConn->setPort1( port1 );
        editedConn->setPort2( nullptr );
        editedConn->updatePosFromPorts( );
        editedConn->setPos2( mousePos );
        editedConn->updatePath( );
        addItem( editedConn );
      } else {
        editedConn = nullptr;
      }
    } else {
      editedConn = new QNEConnection( );
      addItem( editedConn );
      editedConn->setPort1( ( QNEPort* ) item );
      editedConn->setPos1( mousePos );
      editedConn->setPos2( mousePos );
      editedConn->updatePath( );
    }
  } else if( item && ( item->type( ) == QNEConnection::Type ) ) {
    /* Mouse pressed over a connections. */
    QNEConnection *connection = dynamic_cast< QNEConnection* >( item );
    if( connection ) {
      connection->split( mousePos );
    }
  } else if( !item ) {
    if( mouseEvt->button( ) == Qt::LeftButton ) {
      /* Mouse pressed over boar (Selection box). */
      selectionStartPoint = mousePos;
      markingSelectionBox = true;
      selectionRect->setRect( QRectF( selectionStartPoint, selectionStartPoint ) );
      selectionRect->show( );
      selectionRect->update( );
    }
  } else if( item && ( mouseEvt->button( ) == Qt::LeftButton ) ) {
    /* STARTING MOVING ELEMENT */
    draggingElement = true;
    QList< QGraphicsItem* > list = scene->selectedItems( );
    list.append( itemsAt( mousePos ) );
    movedElements.clear( );
    oldPositions.clear( );
    foreach( QGraphicsItem * item, list ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      if( elm ) {
        movedElements.append( elm );
        oldPositions.append( elm->pos( ) );
      }
    }
  }
  return( false );
}

void Editor::handleHoverPort( QNEPort *port ) {
  if( m_hoverPort && ( port != m_hoverPort ) ) {
    releaseHoverPort( );
  } else if( port && ( port->type( ) == QNEPort::Type ) ) {
    m_hoverPort = port;
    if( editedConn && editedConn->port1( ) && ( editedConn->port1( )->isOutput( ) == port->isOutput( ) ) ) {
      QApplication::setOverrideCursor( QCursor( Qt::ForbiddenCursor ) );
    } else {
      m_hoverPort->hoverEnter( );
    }
  }
}

void Editor::releaseHoverPort( ) {
  if( m_hoverPort ) {
    m_hoverPort->hoverLeave( );
    m_hoverPort = nullptr;
    QApplication::setOverrideCursor( QCursor( ) );
  }
}

bool Editor::mouseMoveEvt( QGraphicsSceneMouseEvent *mouseEvt ) {
  Q_UNUSED( mouseEvt )
  QNEPort * port = dynamic_cast< QNEPort* >( itemAt( mousePos ) );
  handleHoverPort( port );
  if( editedConn ) {
    /* If a connection is being created, the ending coordinate follows the mouse position. */
    editedConn->setPos2( mousePos );
    editedConn->updatePath( );
    return( true );
  } else if( markingSelectionBox ) {
    /* If is marking the selectionBox, the last coordinate follows the mouse position. */
    QRectF rect = QRectF( selectionStartPoint, mousePos ).normalized( );
    selectionRect->setRect( rect );
    QPainterPath selectionBox;
    selectionBox.addRect( rect );
    scene->setSelectionArea( selectionBox );
  } else if( !markingSelectionBox ) {
    /* Else, the selectionRect is hidden. */
    selectionRect->hide( );
  }
  return( false );
}

bool Editor::mouseReleaseEvt( QGraphicsSceneMouseEvent *mouseEvt ) {
  releaseHoverPort( );
  /* When mouse is released the selection rect is hidden. */
  selectionRect->hide( );
  markingSelectionBox = false;
  if( QApplication::overrideCursor( ) ) {
    QApplication::setOverrideCursor( Qt::ArrowCursor );
  }
  if( draggingElement && ( mouseEvt->button( ) == Qt::LeftButton ) ) {
    if( !movedElements.empty( ) ) {
      /* FIXME asdusdhuiahdiuashahduis */
      undoStack->push( new MoveCommand( movedElements, oldPositions ) );
    }
    draggingElement = false;
    movedElements.clear( );
  }
  if( editedConn && ( mouseEvt->button( ) == Qt::LeftButton ) ) {
    /* A connection is being created, and left button was released. */
    QNEPort *port = dynamic_cast< QNEPort* >( itemAt( mousePos ) );
    if( port && ( port->type( ) == QNEPort::Type ) ) {
      /* The mouse is released over a QNEPort. */
      QNEPort *port1 = editedConn->port1( );
      QNEPort *port2 = dynamic_cast< QNEPort* >( port );
      if( !port2 ) {
        return( true );
      }
      /* Verifying if the connection is valid. */
      if( ( port1->isOutput( ) != port2->isOutput( ) ) &&
          ( port1->graphicElement( ) != port2->graphicElement( ) ) && !port1->isConnected( port2 ) ) {
        qDebug( ) << "Finalizing connection.";
        /* Finalizing connection. */
        editedConn->setPos2( port2->scenePos( ) );
        editedConn->setPort2( port2 );
        editedConn->updatePath( );
        undoStack->push( new AddItemsCommand( editedConn, this ) );
        editedConn = nullptr;
        return( true );
      }
    }
    /* When mouse is released away from a QNEPort, the connection is discarded. */
    scene->removeItem( editedConn );
    if( editedConn->port1( ) ) {
      editedConn->port1( )->disconnect( editedConn );
    }
    editedConn = nullptr;
    return( true );
  }
  return( false );
}

bool Editor::loadBox( Box *box, QString fname ) {
  try {
    box->setParentFile( GlobalProperties::currentFile );
    box->loadFile( fname );
  } catch( std::runtime_error err ) {
    int ret = QMessageBox::warning( mainWindow, "Error", QString::fromStdString(
                                      err.what( ) ), QMessageBox::Ok, QMessageBox::Cancel );
    if( ret == QMessageBox::Cancel ) {
      return( false );
    } else {
      fname = mainWindow->getOpenBoxFile( );
      if( fname.isEmpty( ) ) {
        return( false );
      } else {
        return( loadBox( box, fname ) );
      }
    }
  }
  return( true );
}

bool Editor::dropEvt( QGraphicsSceneDragDropEvent *dde ) {
  /* Verify if mimetype is compatible. */
  if( dde->mimeData( )->hasFormat( "application/x-dnditemdata" ) ) {
    /* Extracting mimedata from drop event. */
    QByteArray itemData = dde->mimeData( )->data( "application/x-dnditemdata" );
    QDataStream dataStream( &itemData, QIODevice::ReadOnly );
    QPointF offset;
    QString label_auxData;
    qint32 type;
    dataStream >> offset >> type >> label_auxData;
    QPointF pos = dde->scenePos( ) - offset;
    GraphicElement *elm = factory.buildElement( ( ElementType ) type );
    /* If element type is unknown, a default element is created with the pixmap received from mimedata */
    if( !elm ) {
      return( false );
    }
    if( elm->elementType( ) == ElementType::BOX ) {
      Box *box = dynamic_cast< Box* >( elm );
      if( box ) {
        QString fname = label_auxData;
        if( !loadBox( box, fname ) ) {
          return( false );
        }
      }
    }
    /*
     * TODO: Rotate all element icons, remake the port position logic, and remove the code below.
     * Rotating element in 90 degrees.
     */
    if( elm->rotatable( ) ) {
      elm->setRotation( 90 );
    }
    /* Adding the element to the scene. */
    undoStack->push( new AddItemsCommand( elm, this ) );
    /* Cleaning the selection. */
    scene->clearSelection( );
    /* Setting created element as selected. */
    elm->setSelected( true );
    /* Adjusting the position of the element. */
    elm->setPos( pos );
    return( true );
  }
  return( false );
}

bool Editor::dragMoveEvt( QGraphicsSceneDragDropEvent *dde ) {
  /* Accepting drag/drop event of the following mimedata format. */
  if( dde->mimeData( )->hasFormat( "application/x-dnditemdata" ) ) {
    return( true );
  }
  return( false );
}

bool Editor::wheelEvt( QWheelEvent *wEvt ) {
  if( wEvt ) {
    int numDegrees = wEvt->delta( ) / 8;
    int numSteps = numDegrees / 15;
    if( wEvt->orientation( ) == Qt::Horizontal ) {
      emit scroll( numSteps, 0 );
    } else {
      emit scroll( 0, numSteps );
    }
    wEvt->accept( );
    return( true );
  }
  return( false );
}

QUndoStack* Editor::getUndoStack( ) const {
  return( undoStack );
}

ElementFactory &Editor::getFactory( ) {
  return( factory );
}

Scene* Editor::getScene( ) const {
  return( scene );
}

void Editor::cut( QDataStream &ds ) {
  copy( ds );
  deleteElements( );
}

void Editor::copy( QDataStream &ds ) {
  QPointF center( 0.0f, 0.0f );
  float elm = 0;
  foreach( QGraphicsItem * item, scene->selectedItems( ) ) {
    if( item->type( ) == GraphicElement::Type ) {
      center += item->pos( );
      elm++;
    }
  }
  ds << center / elm;
  SerializationFunctions::serialize( scene->selectedItems( ), ds );
}

void Editor::paste( QDataStream &ds ) {
  scene->clearSelection( );
  QPointF ctr;
  ds >> ctr;
  QPointF offset = mousePos - ctr - QPointF( 32.0f, 32.0f );
  QList< QGraphicsItem* > itemList = SerializationFunctions::deserialize( this, ds );
  undoStack->push( new AddItemsCommand( itemList, this ) );
  foreach( QGraphicsItem * item, itemList ) {
    if( item->type( ) == GraphicElement::Type ) {
      item->setPos( ( item->pos( ) + offset ) );
      item->update( );
    }
  }
}

void Editor::selectAll( ) {
  foreach( QGraphicsItem * item, scene->items( ) ) {
    item->setSelected( true );
  }
}

void Editor::save( QDataStream &ds ) {
  ds << QApplication::applicationName( ) + " " + QApplication::applicationVersion( );
  foreach( QGraphicsItem * item, scene->items( ) ) {
    if( item->type( ) == GraphicElement::Type ) {
      ds << item->type( );
      ds << ( quint64 ) ( ( GraphicElement* ) item )->elementType( );
      ( ( GraphicElement* ) item )->save( ds );
    }
  }

  foreach( QGraphicsItem * item, scene->items( ) ) {
    if( item->type( ) == QNEConnection::Type ) {
      ds << item->type( );
      ( ( QNEConnection* ) item )->save( ds );
    }
  }
}

void Editor::load( QDataStream &ds ) {
  /* Any change here must be made in box implementation!!! */
  clear( );
  QString str;
  ds >> str;
  if( !str.startsWith( QApplication::applicationName( ) ) ) {
    throw( std::runtime_error( "Invalid file format." ) );
  } else if( !str.endsWith( QApplication::applicationVersion( ) ) ) {
    QMessageBox::warning(
      dynamic_cast< QWidget* >( parent( ) ), "Warning!", "File opened in compatibility mode.", QMessageBox::Ok,
      QMessageBox::NoButton );
  }
  double version = str.split( " " ).at( 1 ).toDouble( );

  QMap< quint64, QNEPort* > portMap;
  while( !ds.atEnd( ) ) {
    int type;
    ds >> type;
    if( type == GraphicElement::Type ) {
      quint64 elmType;
      ds >> elmType;
      GraphicElement *elm = factory.buildElement( ( ElementType ) elmType );
      if( elm ) {
        addItem( elm );
        elm->load( ds, portMap, version );
      } else {
        throw( std::runtime_error( "Could not build element." ) );
      }
    } else if( type == QNEConnection::Type ) {
      QNEConnection *conn = new QNEConnection( 0 );
      addItem( conn );
      conn->load( ds, portMap );
    } else {
      if( QMessageBox::warning( dynamic_cast< QWidget* >( parent( ) ), "Warning!",
                                "File opened with errors.\nDo you want to proceed?", QMessageBox::Ok,
                                QMessageBox::Cancel ) == QMessageBox::Cancel ) {
        clear( );
      }
      return;
    }
  }
  /* Any change here must be made in box implementation!!! */
}

void Editor::setElementEditor( ElementEditor *value ) {
  elementEditor = value;
  elementEditor->setScene( scene );
  connect( elementEditor, &ElementEditor::elementUpdated, this, &Editor::elementUpdated );
}

QPointF roundTo( QPointF point, int multiple ) {
  int x = static_cast< int >( point.x( ) );
  int y = static_cast< int >( point.y( ) );
  int nx = multiple * qFloor( x / multiple );
  int ny = multiple * qFloor( y / multiple );
  return( QPointF( nx, ny ) );
}

bool Editor::eventFilter( QObject *obj, QEvent *evt ) {
  if (obj == scene ) {
    QGraphicsSceneDragDropEvent *dde = dynamic_cast< QGraphicsSceneDragDropEvent* >( evt );
    QGraphicsSceneMouseEvent *mouseEvt = dynamic_cast< QGraphicsSceneMouseEvent* >( evt );
    QWheelEvent *wEvt = dynamic_cast< QWheelEvent* >( evt );
    if( mouseEvt ) {
      mousePos = mouseEvt->scenePos( );
    }
    bool ret = false;
    switch( ( int ) evt->type( ) ) {
    case QEvent::GraphicsSceneMousePress: {
        ret = mousePressEvt( mouseEvt );
        break;
      }
    case QEvent::GraphicsSceneMouseMove: {
        ret = mouseMoveEvt( mouseEvt );
        break;
      }
    case QEvent::GraphicsSceneMouseRelease: {
        ret = mouseReleaseEvt( mouseEvt );
        break;
      }
    case QEvent::GraphicsSceneDrop: {
        ret = dropEvt( dde );
        break;
      }
    case QEvent::GraphicsSceneDragMove:
    case QEvent::GraphicsSceneDragEnter: {
        ret = dragMoveEvt( dde );
        break;
      }
    case QEvent::GraphicsSceneWheel: {
        ret = wheelEvt( wEvt );
        break;
      }
    }
    if( ret ) {
      return( ret );
    }
  }
  return( QObject::eventFilter( obj, evt ) );
}
