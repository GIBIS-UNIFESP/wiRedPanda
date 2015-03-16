#include "graphicelement.h"

#include <QKeyEvent>

GraphicElement::GraphicElement(QPixmap pixmap, QGraphicsItem *parent) : QGraphicsItem(parent), pixmapItem(new QGraphicsPixmapItem(pixmap, ( QGraphicsItem * ) this)) {
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
}

GraphicElement::~GraphicElement() {

}

QRectF GraphicElement::boundingRect() const {
  return (pixmapItem->boundingRect());
}

void GraphicElement::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  pixmapItem->paint(painter,option,widget);
}

void GraphicElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * ) {
}


void GraphicElement::keyPressEvent(QKeyEvent * event) {
  if(event->key() == Qt::Key_R) {
    setTransformOriginPoint(32,32);
    setRotation(rotation()+90.0);
  }
}
