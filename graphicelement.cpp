#include "graphicelement.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

GraphicElement::GraphicElement(QPixmap pixmap, QGraphicsItem *parent) : QGraphicsItem(parent), pixmapItem(new QGraphicsPixmapItem(pixmap, ( QGraphicsItem * ) this)) {
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);

}

GraphicElement::~GraphicElement() {

}

QRectF GraphicElement::boundingRect() const {
  return (pixmapItem->boundingRect());
}

void GraphicElement::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  Q_UNUSED(option)
  Q_UNUSED(widget)
  if(isSelected()) {
    painter->setBrush(Qt::darkGray);
    painter->setPen(QPen(Qt::black));
    painter->drawRoundedRect(boundingRect(),16,16);
  }
}

QNEPort *GraphicElement::addPort(bool isOutput) {
  QNEPort *port = new QNEPort(this);
  port->setIsOutput(isOutput);
  port->setGraphicElement(this);
  if(isOutput) {
    outputs.push_back(port);
    int x = 32 - outputs.size()*6 + 6;
    foreach (QNEPort * port, outputs) {
      port->setPos(x,64);
      x+= 12;
    }
  } else {
    inputs.push_back(port);
    int x = 32 - inputs.size()*6 + 6;
    foreach (QNEPort * port, inputs) {
      port->setPos(x,0);
      x+= 12;
    }
  }
  port->show();
  return port;
}

void GraphicElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * ) {
//  addPort(true);
  addPort(false);
}

QVariant GraphicElement::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value) {
  if(change == ItemScenePositionHasChanged ||  change == ItemRotationHasChanged ||  change == ItemTransformHasChanged) {
    foreach (QNEPort * port, outputs) {
      port->updateConnections();
    }
    foreach (QNEPort * port, inputs) {
      port->updateConnections();
    }
  }
  return value;
}
