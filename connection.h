#ifndef CONNECTION_H
#define CONNECTION_H

#include <QGraphicsItem>

class Connection :QGraphicsItem {
public:
  Connection(QGraphicsItem *parent );
  ~Connection();
  QPointF getPos() const;
  void setPos(const QPointF & value);

private:
  QPointF pos;
  // QGraphicsItem interface
public:
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *);
};

#endif // CONNECTION_H
