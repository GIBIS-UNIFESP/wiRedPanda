#include "connection.h"

#include <QDebug>
#include <QPainter>

Connection::Connection(QGraphicsItem * parent) : QGraphicsItem(parent), pos(32,60) {
  setFlag(QGraphicsItem::ItemIsSelectable, true);
//  setFlag(QGraphicsItem::ItemIsMovable, true);
}

Connection::~Connection() {

}

QRectF Connection::boundingRect() const {
  return QRectF(pos - QPointF(4,4),pos + QPointF(4,4));
}

QPointF Connection::getPos() const {
  return pos;
}

void Connection::setPos(const QPointF & value) {
  pos = value;
  update();
}

void Connection::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *x) {
  if(isSelected()){
    painter->setPen(QPen(Qt::black,2,Qt::SolidLine));
    painter->setBrush(QBrush(Qt::green,Qt::SolidPattern));
    painter->drawEllipse(pos,4,4);
  }else{
    painter->setPen(QPen(Qt::black,2,Qt::SolidLine));
    painter->setBrush(QBrush(Qt::red,Qt::SolidPattern));
    painter->drawEllipse(pos,4,4);
  }
//  painter->drawRect(boundingRect());
  qDebug() << "Drawing : " << pos;
}
