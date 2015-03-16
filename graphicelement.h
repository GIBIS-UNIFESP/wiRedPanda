#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>

class GraphicElement : public QGraphicsItem {
public:
  explicit GraphicElement(QPixmap pixmap, QGraphicsItem * parent = 0);
  ~GraphicElement();

private:
  QGraphicsPixmapItem *pixmapItem;

  // QGraphicsItem interface
public:
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

  // QGraphicsItem interface
  protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);

  // QGraphicsItem interface
  protected:
  virtual void keyPressEvent(QKeyEvent * event);
};

#endif // GRAPHICELEMENT_H
