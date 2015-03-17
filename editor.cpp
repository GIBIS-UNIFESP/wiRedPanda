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
  QKeyEvent * keyEvt = dynamic_cast<QKeyEvent *>(e);
  switch ((int) e->type()) {
  case QEvent::KeyPress: {
      qDebug() << "KeyPress";
      switch ((int) keyEvt->key()) {
      case Qt::Key_Delete:
      case Qt::Key_Backspace: {
          foreach(QGraphicsItem * item, scene->selectedItems()) {
            scene->removeItem(item);
            delete item;
          }
          return true;
          break;
        }
      case Qt::Key_R: {
          double cx = 0, cy = 0;
          int sz = 0;
          QList<QGraphicsItem *> list = scene->selectedItems();
          foreach(QGraphicsItem * item, list) {
            qDebug() <<"Before: " << item->scenePos();
            qDebug() <<"Before: " << item->pos();
            item->setTransformOriginPoint(32,32);
            qDebug() << "After: " << item->scenePos();
            qDebug() << "After: " << item->pos();

            cx += item->pos().x();
            cy += item->pos().y();

            sz ++;
          }
          cx /= sz;
          cy /= sz;
          qDebug() << cx << cy;
          foreach(QGraphicsItem * item, list) {
            QTransform transform;
            transform.translate( cx , cy  );
            transform.rotate( 90 );
            transform.translate( -cx , -cy );
            item->resetTransform();
            item->setRotation(item->rotation()+90.0);
            item->resetTransform();
            item->setPos(transform.map(item->pos()));
            qDebug() << item->transformOriginPoint();
          }
          break;
        }
      }
      break;
    }
  case QEvent::GraphicsSceneMousePress: {
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
          qDebug() << item->type() << QNEPort::Type;
          QNEPort *port1 = conn->port1();
          QNEPort *port2 = dynamic_cast<QNEPort*> (item);
          if(!port2) {
            return true;
          }
          if (port1->graphicElement() != port2->graphicElement() && !port1->isConnected(port2)) {
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
        qDebug() << "Drop Event";
        QByteArray itemData = dde->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QPointF offset;
        dataStream >> pixmap >> offset;
        QPointF pos = dde->scenePos() - offset;
//        pos = roundTo(pos,64);
//        qDebug() << pos << roundTo(pos,64);
        GraphicElement * item = new GraphicElement(pixmap);
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
