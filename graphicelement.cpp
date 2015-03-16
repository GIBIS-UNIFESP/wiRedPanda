#include "graphicelement.h"

#include <QKeyEvent>
#include <QMessageBox>

GraphicElement::GraphicElement(QPixmap pixmap, QGraphicsItem *parent) : QGraphicsItem(parent), pixmapItem(new QGraphicsPixmapItem(pixmap, ( QGraphicsItem * ) this)) {
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
//  setFlag(QGraphicsItem::ItemIsFocusable, true);
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
  QMessageBox::information(0,"WPANDA","<strong>Item double clicked!</strong><br>Menu enters here.",QMessageBox::Ok, QMessageBox::NoButton);
}
