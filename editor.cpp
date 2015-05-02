#include "editor.h"
#include "graphicelement.h"

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QLabel>
#include <QtMath>
#include <iostream>
#include <QKeyEvent>

#include <nodes/qneconnection.h>

Editor::Editor(QObject *parent) : QObject(parent), scene(NULL), conn(NULL) {
  markingSelectionBox = false;
  selectionRect = new QGraphicsRectItem();
  selectionRect->setBrush(Qt::NoBrush);
  selectionRect->setFlag(QGraphicsItem::ItemIsSelectable, false);
  selectionRect->setPen(QPen(Qt::darkGray,1.5,Qt::DotLine));
}

Editor::~Editor() {

}

void Editor::install(QGraphicsScene * s) {
  s->installEventFilter(this);
  scene = s;
  scene->addItem(selectionRect);
  simulationController = new SimulationController(s);
  simulationController->start();
}

void Editor::clear() {
  foreach (QGraphicsItem * item, scene->items()) {
    if(!scene->items().contains(item))
      continue;
    scene->removeItem(item);
//    delete item;
  }
  scene->clear();
  scene->addItem(selectionRect);
}

void Editor::deleteElements() {
  foreach(QGraphicsItem * item, scene->selectedItems()) {
    if(!scene->items().contains(item))
      continue;
    scene->removeItem(item);
    delete item;
  }
}

void Editor::showWires(bool checked) {

  foreach( QGraphicsItem * c, scene->items()) {
    if(c->type() == QNEConnection::Type) {
      c->setVisible(checked);
    }
  }
}

void Editor::rotate(bool rotateRight) {
  double angle = 90.0;
  if(!rotateRight) {
    angle = -angle;
  }
  double cx = 0, cy = 0;
  int sz = 0;
  QList<QGraphicsItem *> list = scene->selectedItems();
  foreach(QGraphicsItem * item, list) {

    if(item->type() == GraphicElement::Type) {
      item->setTransformOriginPoint(32,32);

      cx += item->pos().x();
      cy += item->pos().y();

      sz ++;
    }
  }
  cx /= sz;
  cy /= sz;
  foreach(QGraphicsItem * item, list) {
    if(item->type() == GraphicElement::Type) {
      GraphicElement * elm = qgraphicsitem_cast<GraphicElement *>(item);
      QTransform transform;
      transform.translate( cx , cy  );
      transform.rotate( angle );
      transform.translate( -cx , -cy );
      if(elm) {
        if(elm->rotatable()) {
          elm->setRotation(item->rotation()+angle);
        }
      }
      item->setPos(transform.map(item->pos()));
    }
  }
}

QGraphicsItem *Editor::itemAt(const QPointF & pos) {
  QList<QGraphicsItem*> items = scene->items(QRectF(pos - QPointF(1,1), QSize(3,3)).normalized());

  foreach(QGraphicsItem *item, items) {
    if (item->type() > QGraphicsItem::UserType)
      return item;
  }

  return 0;
}

void Editor::setElementEditor(ElementEditor * value) {
  elementEditor = value;
}

QPointF roundTo(QPointF point, int multiple) {
  int x = static_cast<int>(point.x());
  int y = static_cast<int>(point.y());
  int nx = multiple * qFloor(x/multiple);
  int ny = multiple * qFloor(y/multiple);
  return( QPointF(nx,ny));
}

bool Editor::eventFilter(QObject * o, QEvent * e) {
  QGraphicsSceneDragDropEvent * dde = dynamic_cast<QGraphicsSceneDragDropEvent *>(e);
  QGraphicsSceneMouseEvent *me = dynamic_cast<QGraphicsSceneMouseEvent*>(e);
  switch ((int) e->type()) {
    //Mouse press event
  case QEvent::GraphicsSceneMousePress: {
      if(!me) {
        break;
      }
      QGraphicsItem * item = itemAt(me->scenePos());
      if( item && item->type()== QNEPort::Type) {
        //Mouse pressed over an item.
        conn = new QNEConnection();
        scene->addItem(conn);
        conn->setPort1((QNEPort*) item);
        conn->setPos1(me->scenePos());
        conn->setPos2(me->scenePos());
        conn->updatePath();
      } else if( item && item->type() == QNEConnection::Type) {
        //Mouse pressed over a connections.
        QNEConnection * connection = dynamic_cast<QNEConnection*>(item);
        if(connection) {
          connection->split(me->scenePos());
        }
      } else if(!item) {
        //Mouse pressed over boar (Selection box).
        selectionStartPoint = me->scenePos();
        markingSelectionBox = true;
        selectionRect->setRect(QRectF(selectionStartPoint,selectionStartPoint));
        selectionRect->show();
      }
      break;
    }
    //Mouse move event.
  case QEvent::GraphicsSceneMouseMove: {
      if (conn) {
        //If a connection is being created, the ending coordinate follows the mouse position.
        conn->setPos2(me->scenePos());
        conn->updatePath();
        return true;
      } else if(markingSelectionBox) {
        //If is marking the selectionBox, the last coordinate follows the mouse position.
        QRectF rect = QRectF(selectionStartPoint,me->scenePos()).normalized();
        selectionRect->setRect(rect);
        QPainterPath selectionBox;
        selectionBox.addRect(rect);
        scene->setSelectionArea(selectionBox);
      } else if(!markingSelectionBox) {
        //Else, the selectionRect is hidden.
        selectionRect->hide();
      }
      break;
    }
    //Mouse release event.
  case QEvent::GraphicsSceneMouseRelease: {
      //When mouse is released the selection rect is hidden.
      selectionRect->hide();
      markingSelectionBox = false;

      if (conn && me->button() == Qt::LeftButton) {
        //A connection is being created, and left button was released.
        QNEPort *item = dynamic_cast<QNEPort*>(itemAt(me->scenePos()));
        if (item && item->type() == QNEPort::Type) {
          //The mouse is released over a QNEPort.
          QNEPort *port1 = conn->port1();
          QNEPort *port2 = dynamic_cast<QNEPort*> (item);
          if(!port2) {
            return true;
          }
          //Verifying if the connection is valid.
          if (port1->isOutput() != port2->isOutput() && port1->graphicElement() != port2->graphicElement() && !port1->isConnected(port2)) {
            //Finalizing connection.
            conn->setPos2(port2->scenePos());
            conn->setPort2(port2);
            conn->updatePath();
            conn = 0;
            return true;
          }
        }
        //When mouse is released away from a QNEPort, the connection is discarded.
        delete conn;
        conn = NULL;
        return true;
      }
      break;
    }
    //Drop event.
  case QEvent::GraphicsSceneDrop: {
      //Verify if mimetype is compatible.
      if(dde->mimeData()->hasFormat("application/x-dnditemdata")) {
        //Extracting mimedata from drop event.
        QByteArray itemData = dde->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPixmap pixmap;
        QPointF offset;
        qint32 type;
        dataStream >> pixmap >> offset >> type;
        QPointF pos = dde->scenePos() - offset;
        qDebug() << "Drop event: " << pixmap << " , " << offset << " , " << type;

        //        pos = roundTo(pos,64);
        //        qDebug() << pos << roundTo(pos,64);
        //Send element type to element factory. Returns NULL if type is unknown.
        GraphicElement * elm = factory.buildElement((ElementType)type);
        //If element type is unknown, a default element is created with the pixmap received from mimedata
        //TODO: Remove this option once all the ports are implemented.
        if(!elm) {
//          break;
          qDebug() << "Unknown port type. Building default element.";
          elm = new GraphicElement(pixmap);
        }
        elm->setTransformOriginPoint(32,32);
        //TODO: Rotate all element icons, remake the port position logic, and remove the code below.
        //Rotating element in 90 degrees.
        if(elm->rotatable()) {
          elm->setRotation(90);
        }
        //Adding the element to the scene.
        scene->addItem(elm);
        //Cleaning the selection.
        scene->clearSelection();
        //Setting created element as selected.
        elm->setSelected(true);
        //Adjusting the position of the element.
        elm->setPos(pos);
        return true;
      }
      break;
    }
  case QEvent::GraphicsSceneDragMove:
  case QEvent::GraphicsSceneDragEnter: {
      //Accepting drag/drop event of the following mimedata format.
      if(dde->mimeData()->hasFormat("application/x-dnditemdata")) {
        return true;
      }
      break;
    }
  }
  return QObject::eventFilter(o, e);
}
