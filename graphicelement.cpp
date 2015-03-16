#include "graphicelement.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

GraphicElement::GraphicElement(QPixmap pixmap, QGraphicsItem *parent) : QGraphicsItem(parent), pixmapItem(new QGraphicsPixmapItem(pixmap, ( QGraphicsItem * ) this)) {
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
//  pixmapItem->hide();
//  setFlag(QGraphicsItem::ItemIsFocusable, true);
}

GraphicElement::~GraphicElement() {

}

QRectF GraphicElement::boundingRect() const {
  return (pixmapItem->boundingRect());
}

void GraphicElement::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  pixmapItem->paint(painter,option,widget);
  int start = 32 - 5*inputs.size() + 5;
  int end = 32 + 5*inputs.size();
  qDebug( ) << "inputsz = " << inputs.size();
  int pos = 0;
  for(int x = start; x < end; x+= 10) {
    inputs.at(pos++)->setPos(QPointF(x,60));
  }
  pos = 0;
  start = 32 - 5 * outputs.size() + 5;
  end   = 32 + 5 * outputs.size();
  for(int x = start; x < end; x+= 10) {
    outputs.at(pos++)->setPos(QPointF(x,4));
  }
}

void GraphicElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * ) {
//  QMessageBox::information(0,"WPANDA","<strong>Item double clicked!</strong><br>Menu enters here.",QMessageBox::Ok, QMessageBox::NoButton);
//  if(inputs.size()<= outputs.size())
    inputs.push_back(new Connection(this));
//  else
//    outputs.push_back(new Connection(this));
  update();
}
