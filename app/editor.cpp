#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"
#include "serializationfunctions.h"

#include <QApplication>
#include <QDebug>
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

Editor::Editor( QObject *parent ) : QObject( parent ), scene( NULL ), conn( NULL ), m_hoverPort( NULL ) {
  markingSelectionBox = false;
  undoStack = new QUndoStack( this );
  scene = new Scene( this );
  scene->setBackgroundBrush( QBrush( QColor( Qt::gray ) ) );
  scene->setGridSize( 16 );
  install( scene );
  buildSelectionRect( );
}

Editor::~Editor( ) {
}

void Editor::install( Scene *s ) {
  s->installEventFilter( this );
  addItem( selectionRect );
  simulationController = new SimulationController( s );
  simulationController->start( );
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
  double cx = 0, cy = 0;
  int sz = 0;
  QList< QGraphicsItem* > list = scene->selectedItems( );
  foreach( QGraphicsItem * item, list ) {
    if( item->type( ) == GraphicElement::Type ) {
      item->setTransformOriginPoint( 32, 32 );

      cx += item->pos( ).x( );
      cy += item->pos( ).y( );

      sz++;
    }
  }
  cx /= sz;
  cy /= sz;
  foreach( QGraphicsItem * item, list ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      QTransform transform;
      transform.translate( cx, cy );
      transform.rotate( angle );
      transform.translate( -cx, -cy );
      if( elm ) {
        if( elm->rotatable( ) ) {
          elm->setRotation( item->rotation( ) + angle );
        }
      }
      item->setPos( transform.map( item->pos( ) ) );
    }
    item->update();
  }
}

QGraphicsItem* Editor::itemAt( const QPointF &pos ) {
  QList< QGraphicsItem* > items = scene->items( QRectF( pos - QPointF( 7, 7 ), QSize( 13, 13 ) ).normalized( ) );

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

  return( 0 );
}

void Editor::addItem( QGraphicsItem *item ) {
  scene->addItem( item );
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
      item->update();
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
  }
  else if( !str.endsWith( QApplication::applicationVersion( ) ) ) {
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
      }
      else {
        throw( std::runtime_error( "Could not build element." ) );
      }
    }
    else if( type == QNEConnection::Type ) {
      QNEConnection *conn = new QNEConnection( 0 );
      addItem( conn );
      conn->load( ds, portMap );
    }
    else {
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
}

QPointF roundTo( QPointF point, int multiple ) {
  int x = static_cast< int >( point.x( ) );
  int y = static_cast< int >( point.y( ) );
  int nx = multiple * qFloor( x / multiple );
  int ny = multiple * qFloor( y / multiple );
  return( QPointF( nx, ny ) );
}

bool Editor::eventFilter( QObject *obj, QEvent *evt ) {
  QGraphicsSceneDragDropEvent *dde = dynamic_cast< QGraphicsSceneDragDropEvent* >( evt );
  QGraphicsSceneMouseEvent *mouseEvt = dynamic_cast< QGraphicsSceneMouseEvent* >( evt );
  if( mouseEvt ) {
    mousePos = mouseEvt->scenePos( );
  }
  switch( ( int ) evt->type( ) ) {
      /* Mouse press event */
      case QEvent::GraphicsSceneMousePress: {
      if( m_hoverPort ) {
        m_hoverPort->hoverLeave( );
        m_hoverPort = NULL;
        QApplication::setOverrideCursor( QCursor( ) );
      }
      if( mouseEvt ) {
        QGraphicsItem *item = itemAt( mousePos );
        if( item && ( item->type( ) == QNEPort::Type ) ) {
          /* Mouse pressed over an item. */
          QNEPort *port2 = ( QNEPort* ) item;
          if( port2->isInput( ) && !port2->connections( ).empty( ) ) {
            conn = port2->connections( ).last( );
            conn->setFocus( );
            QNEPort *port1 = conn->otherPort( port2 );
            if( port1 ) {
              port2->disconnect( conn );
              conn->setPort2( NULL );
              conn->setPort1( port1 );
              conn->setPos1( port1->scenePos( ) );
              conn->setPos2( mousePos );
              conn->updatePath( );
            }
            else {
              conn = NULL;
            }
          }
          else {
            conn = new QNEConnection( );
            addItem( conn );
            conn->setPort1( ( QNEPort* ) item );
            conn->setPos1( mousePos );
            conn->setPos2( mousePos );
            conn->updatePath( );
          }
        }
        else if( item && ( item->type( ) == QNEConnection::Type ) ) {
          /* Mouse pressed over a connections. */
          QNEConnection *connection = dynamic_cast< QNEConnection* >( item );
          if( connection ) {
            connection->split( mousePos );
          }
        }
        else if( !item ) {
          if( mouseEvt->button( ) == Qt::LeftButton ) {
            /* Mouse pressed over boar (Selection box). */
            selectionStartPoint = mousePos;
            markingSelectionBox = true;
            selectionRect->setRect( QRectF( selectionStartPoint, selectionStartPoint ) );
            selectionRect->show( );
            selectionRect->update();
          }
        }
      }
      break;
    }
      /* Mouse move event. */
      case QEvent::GraphicsSceneMouseMove: {
      QNEPort *item = dynamic_cast< QNEPort* >( itemAt( mousePos ) );
      if( m_hoverPort && ( item != m_hoverPort ) ) {
        m_hoverPort->hoverLeave( );
        m_hoverPort = NULL;
        QApplication::setOverrideCursor( QCursor( ) );
      }
      else if( item && ( item->type( ) == QNEPort::Type ) ) {
        m_hoverPort = item;
        if( conn && conn->port1( ) && ( conn->port1( )->isOutput( ) == item->isOutput( ) ) ) {
          QApplication::setOverrideCursor( QCursor( Qt::ForbiddenCursor ) );
        }
        else {
          m_hoverPort->hoverEnter( );
        }
      }
      if( conn ) {
        /* If a connection is being created, the ending coordinate follows the mouse position. */
        conn->setPos2( mousePos );
        conn->updatePath( );
        return( true );
      }
      else if( markingSelectionBox ) {
        /* If is marking the selectionBox, the last coordinate follows the mouse position. */
        QRectF rect = QRectF( selectionStartPoint, mousePos ).normalized( );
        selectionRect->setRect( rect );
        QPainterPath selectionBox;
        selectionBox.addRect( rect );
        scene->setSelectionArea( selectionBox );
      }
      else if( !markingSelectionBox ) {
        /* Else, the selectionRect is hidden. */
        selectionRect->hide( );
      }
      break;
    }
      /* Mouse release event. */
      case QEvent::GraphicsSceneMouseRelease: {
      if( m_hoverPort ) {
        m_hoverPort->hoverLeave( );
        m_hoverPort = NULL;
        QApplication::setOverrideCursor( QCursor( ) );
      }
      /* When mouse is released the selection rect is hidden. */
      selectionRect->hide( );
      markingSelectionBox = false;
      if( QApplication::overrideCursor( ) ) {
        QApplication::setOverrideCursor( Qt::ArrowCursor );
      }
      if( conn && ( mouseEvt->button( ) == Qt::LeftButton ) ) {
        /* A connection is being created, and left button was released. */
        QNEPort *item = dynamic_cast< QNEPort* >( itemAt( mousePos ) );
        if( item && ( item->type( ) == QNEPort::Type ) ) {
          /* The mouse is released over a QNEPort. */
          QNEPort *port1 = conn->port1( );
          QNEPort *port2 = dynamic_cast< QNEPort* >( item );
          if( !port2 ) {
            return( true );
          }
          /* Verifying if the connection is valid. */
          if( ( port1->isOutput( ) != port2->isOutput( ) ) &&
              ( port1->graphicElement( ) != port2->graphicElement( ) ) && !port1->isConnected( port2 ) ) {
            /* Finalizing connection. */
            conn->setPos2( port2->scenePos( ) );
            conn->setPort2( port2 );
            conn->updatePath( );
            undoStack->push( new AddItemsCommand( conn, this ) );
            conn = nullptr;
            return( true );
          }
        }
        /* When mouse is released away from a QNEPort, the connection is discarded. */
        delete conn;
        conn = NULL;
        return( true );
      }
      break;
    }
      /* Drop event. */
      case QEvent::GraphicsSceneDrop: {
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
/*        qDebug() << "Drop event: " << " , " << offset << " , " << type; */

        /*
         *        pos = roundTo(pos,64);
         *        qDebug() << pos << roundTo(pos,64);
         * Send element type to element factory. Returns NULL if type is unknown.
         */
        GraphicElement *elm = factory.buildElement( ( ElementType ) type );
        /* If element type is unknown, a default element is created with the pixmap received from mimedata */
        if( !elm ) {
          return( false );
        }
        if( elm->elementType( ) == ElementType::BOX ) {
          Box *box = dynamic_cast< Box* >( elm );
          if( box ) {
            box->loadFile( label_auxData );
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
      break;
    }
      case QEvent::GraphicsSceneDragMove:
      case QEvent::GraphicsSceneDragEnter: {
      /* Accepting drag/drop event of the following mimedata format. */
      if( dde->mimeData( )->hasFormat( "application/x-dnditemdata" ) ) {
        return( true );
      }
      break;
    }
      case QEvent::GraphicsSceneWheel: {
      QWheelEvent *wheelEvt = dynamic_cast< QWheelEvent* >( evt );
      if( wheelEvt ) {
        int numDegrees = wheelEvt->delta( ) / 8;
        int numSteps = numDegrees / 15;
        if( wheelEvt->orientation( ) == Qt::Horizontal ) {
          emit scroll( numSteps, 0 );
        }
        else {
          emit scroll( 0, numSteps );
        }
        wheelEvt->accept( );
        return( true );
      }
      break;
    }
  }
  return( QObject::eventFilter( obj, evt ) );
}
