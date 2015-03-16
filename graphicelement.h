#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>

#include "connection.h"

class GraphicElement : public QGraphicsItem {
public:
  explicit GraphicElement(QPixmap pixmap, QGraphicsItem * parent = 0);
  ~GraphicElement();

private:
  QGraphicsPixmapItem *pixmapItem;
  QVector<Connection*> inputs;
  QVector<Connection*> outputs;
  // QGraphicsItem interface
public:
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

  // QGraphicsItem interface
  protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
};

#endif // GRAPHICELEMENT_H
