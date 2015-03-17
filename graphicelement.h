#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>

#include "nodes/qneport.h"

class GraphicElement : public QGraphicsItem {
public:
  enum { Type = QGraphicsItem::UserType + 2 };

  explicit GraphicElement(QPixmap pixmap, QGraphicsItem * parent = 0);
  ~GraphicElement();

private:
  QGraphicsPixmapItem *pixmapItem;
  QVector<QNEPort*> inputs;
  QVector<QNEPort*> outputs;
  // QGraphicsItem interface
public:
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
  QNEPort * addPort(bool isOutput);
  // QGraphicsItem interface
protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

#endif // GRAPHICELEMENT_H
