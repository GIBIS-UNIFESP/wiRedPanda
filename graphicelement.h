#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>

#include "nodes/qneport.h"

enum class ElementType {
  EMPTY, UNKNOWN, BUTTON, SWITCH, LED, NOT, AND, OR, NAND, NOR, CLOCK,
  WIRE, DLATCH, SRLATCH, SCRLATCH
};

class GraphicElement : public QObject, public QGraphicsItem {
  Q_OBJECT
public:
  enum { Type = QGraphicsItem::UserType + 3 };

  explicit GraphicElement(QPixmap pixmap, QGraphicsItem * parent = 0);
  explicit GraphicElement(int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem * parent = 0);
  ~GraphicElement();

private:
  QGraphicsPixmapItem *pixmapItem;
  int m_id;
  // QGraphicsItem interface
public:
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
  QNEPort * addPort(bool isOutput);
  // QGraphicsItem interface

  int type() const {
    return Type;
  }

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

  int minInputSz() const;
  void setMinInputSz(int minInputSz);

  int minOutputSz() const;
  void setMinOutputSz(int minOutputSz);

  int id() const;
  void setId(int value);

  void setPixmap(const QPixmap &pixmap);

  void updatePorts();

  bool rotatable() const;
  void setRotatable(bool rotatable);

  protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);
  int m_topPosition;
  int m_bottomPosition;
  int m_maxInputSz;
  int m_maxOutputSz;
  int m_minInputSz;
  int m_minOutputSz;
  bool m_outputsOnTop;
  bool m_rotatable;
  QVector<QNEPort*> m_inputs;
  QVector<QNEPort*> m_outputs;
};

#endif // GRAPHICELEMENT_H
