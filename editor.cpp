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
}

Editor::~Editor() {

}

void Editor::install(QGraphicsScene * s) {
  s->installEventFilter(this);
  scene = s;
}

void Editor::clear() {
  scene->clear();
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
  QList<QGraphicsItem*> items = scene->items(QRectF(pos - QPointF(1,1), QSize(3,3)));

  foreach(QGraphicsItem *item, items) {
    if (item->type() > QGraphicsItem::UserType)
      return item;
  }

  return 0;
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
  case QEvent::GraphicsSceneMousePress: {
      if(!me) {
        break;
      }
      QGraphicsItem * item = itemAt(me->scenePos());
      if( item && item->type()== QNEPort::Type) {
        conn = new QNEConnection();
        scene->addItem(conn);
        conn->setPort1((QNEPort*) item);
        conn->setPos1(me->scenePos());
        conn->setPos2(me->scenePos());
        conn->updatePath();
      } else if( item && item->type() == QNEConnection::Type) {
        QNEConnection * connection = dynamic_cast<QNEConnection*>(item);
        if(connection) {
          connection->split(me->scenePos());
        }
      }
      break;
    }
  case QEvent::GraphicsSceneMouseMove: {
      if (conn) {
        conn->setPos2(me->scenePos());
        conn->updatePath();
        return true;
      }
      break;
    }
  case QEvent::GraphicsSceneMouseRelease: {
      if (conn && me->button() == Qt::LeftButton) {
        QNEPort *item = dynamic_cast<QNEPort*>(itemAt(me->scenePos()));
        if (item && item->type() == QNEPort::Type) {
          QNEPort *port1 = conn->port1();
          QNEPort *port2 = dynamic_cast<QNEPort*> (item);
          if(!port2) {
            return true;
          }
          if (port1->isOutput() != port2->isOutput() && port1->graphicElement() != port2->graphicElement() && !port1->isConnected(port2)) {
            conn->setPos2(port2->scenePos());
            conn->setPort2(port2);
            conn->updatePath();
            conn = 0;
            return true;
          }
        }

        delete conn;
        conn = NULL;
        return true;
      }
      break;
    }
  case QEvent::GraphicsSceneDrop: {
      if(dde->mimeData()->hasFormat("application/x-dnditemdata")) {
        QByteArray itemData = dde->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QPointF offset;
        qint32 type;
        dataStream >> pixmap >> offset >> type;
        QPointF pos = dde->scenePos() - offset;
//        pos = roundTo(pos,64);
//        qDebug() << pos << roundTo(pos,64);
        GraphicElement * item = factory.buildElement((ElementType)type);
        if(!item) {
          item = new GraphicElement(pixmap);
        }
        scene->addItem(item);
        scene->clearSelection();
        item->setSelected(true);
        item->setPos(pos);
        return true;
      }
      break;
    }
  case QEvent::GraphicsSceneDragMove:
  case QEvent::GraphicsSceneDragEnter: {
      if(dde->mimeData()->hasFormat("application/x-dnditemdata")) {
        return true;
      }
      break;
    }
  }
  return QObject::eventFilter(o, e);
}
