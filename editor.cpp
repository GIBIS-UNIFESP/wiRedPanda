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

Editor::Editor(QObject *parent) : QObject(parent), scene(NULL) {
}

Editor::~Editor() {

}

void Editor::install(QGraphicsScene * s) {
  s->installEventFilter(this);
  scene = s;
}

QGraphicsItem *Editor::itemAt(const QPointF & pos) {
  QList<QGraphicsItem*> items = scene->items(QRectF(pos - QPointF(1,1), QSize(3,3)));

  foreach(QGraphicsItem *item, items)
  if (item->type() > QGraphicsItem::UserType)
    return item;

  return 0;
}

QPointF roundTo(QPointF point, int multiple) {
  int x = static_cast<int>(point.x()) - 32;
  int y = static_cast<int>(point.y()) - 32;
  int nx = multiple * qFloor(x/multiple);
  int ny = multiple * qFloor(y/multiple);
  return( QPointF(nx,ny));
}

bool Editor::eventFilter(QObject * o, QEvent * e) {
  QGraphicsSceneDragDropEvent * dde = (QGraphicsSceneDragDropEvent *)e;
  QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent*) e;

  switch ((int) e->type()) {
  case QEvent::GraphicsSceneMousePress: {

      break;
    }
  case QEvent::GraphicsSceneMouseMove: {
      break;
    }
  case QEvent::GraphicsSceneMouseRelease: {
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
        pos = roundTo(pos,64);
        qDebug() << pos << roundTo(pos,64);
        GraphicElement * item = new GraphicElement(pixmap);
        scene->addItem(item);
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
