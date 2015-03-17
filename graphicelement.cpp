#include "graphicelement.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

GraphicElement::GraphicElement(QPixmap pixmap, QGraphicsItem *parent) : QGraphicsItem(parent), pixmapItem(new QGraphicsPixmapItem(pixmap, ( QGraphicsItem * ) this)) {
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);

}

GraphicElement::~GraphicElement() {

}

QRectF GraphicElement::boundingRect() const {
  return (pixmapItem->boundingRect());
}

void GraphicElement::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  Q_UNUSED(option)
  Q_UNUSED(widget)
  Q_UNUSED(painter)

}

QNEPort *GraphicElement::addPort(bool isOutput) {
  QNEPort *port = new QNEPort(this);
  port->setIsOutput(isOutput);
  port->setGraphicElement(this);
  outputs.push_back(port);
  int x = 32 - outputs.size()*6 + 6;
  foreach (QNEPort * port, outputs) {
    port->setPos(x,64);
    x+= 12;
  }
  port->show();
  qDebug() << "Added new port";
  return port;
}

void GraphicElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * ) {
  addPort(true);
}

QVariant GraphicElement::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value) {
  Q_UNUSED(change);

  return value;
}
