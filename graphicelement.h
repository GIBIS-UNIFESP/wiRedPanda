#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>

#include "nodes/qneport.h"

class GraphicElement : public QGraphicsItem {
public:
  enum { Type = QGraphicsItem::UserType + 3 };

  explicit GraphicElement(QPixmap pixmap, QGraphicsItem * parent = 0);
  ~GraphicElement();

private:
  QGraphicsPixmapItem *pixmapItem;
  // QGraphicsItem interface
public:
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
  QNEPort * addPort(bool isOutput);
  // QGraphicsItem interface

  int type() const { return Type; }

  int topPosition() const;
  void setTopPosition(int topPosition);

  int bottomPosition() const;
  void setBottomPosition(int bottomPosition);

  int maxInputSz() const;
  void setMaxInputSz(int maxInputSz);

  int maxOutputSz() const;
  void setMaxOutputSz(int maxOutputSz);

  bool outputsOnTop() const;
  void setOutputsOnTop(bool outputsOnTop);

  QVector<QNEPort *> inputs() const;
  void setInputs(const QVector<QNEPort *> & inputs);

  QVector<QNEPort *> outputs() const;
  void setOutputs(const QVector<QNEPort *> & outputs);

  protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);
  int m_topPosition;
  int m_bottomPosition;
  int m_maxInputSz;
  int m_maxOutputSz;
  bool m_outputsOnTop;
  QVector<QNEPort*> m_inputs;
  QVector<QNEPort*> m_outputs;
};

#endif // GRAPHICELEMENT_H
