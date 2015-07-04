#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>

#include "nodes/qneport.h"

#include "priorityelement.h"

enum class ElementType {
  UNKNOWN, BUTTON, SWITCH, LED, NOT, AND, OR, NAND, NOR, CLOCK, XOR, XNOR, VCC, GND, DISPLAY,
  DLATCH, JKLATCH, DFLIPFLOP, JKFLIPFLOP, SRFLIPFLOP, TFLIPFLOP, TLATCH
};

class GraphicElement : public QGraphicsObject, public PriorityElement {
  Q_OBJECT
public:
  enum { Type = QGraphicsItem::UserType + 3 };

  explicit GraphicElement(int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem * parent = 0);
  ~GraphicElement();

private:
  QPixmap pixmap;
  int m_id;

  //GraphicElement interface.
public:
  virtual ElementType elementType() {
    return ElementType::UNKNOWN;
  }

  virtual void save(QDataStream&ds);

  virtual void load(QDataStream&ds, QMap<quint64, QNEPort*> &portMap, double version);

  virtual void updatePorts();

  virtual void updateLogic() = 0;

  // QGraphicsItem interface
public:

  int type() const {
    return Type;
  }

  virtual QRectF boundingRect() const;

  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

  QNEPort* addPort(const QString &name , bool isOutput, int flags = 0, int ptr = 0);

  void addInputPort(const QString &name = QString());

  void addOutputPort(const QString &name = QString());


  int topPosition() const;

  int bottomPosition() const;

  int maxInputSz() const;

  int maxOutputSz() const;

  bool outputsOnTop() const;

  QVector<QNEPort *> inputs() const;
  void setInputs(const QVector<QNEPort *> & inputs);

  QVector<QNEPort *> outputs() const;
  void setOutputs(const QVector<QNEPort *> & outputs);

  int minInputSz() const;

  int minOutputSz() const;

  int inputSize();
  void setInputSize(int size);

  int outputSize();
  void setOutputSize(int size);

  virtual float frequency();
  virtual void setFrequency(float freq);

  int id() const;
  void setId(int value);

  void setPixmap(const QPixmap &pixmap);

  bool rotatable() const;

  bool hasLabel() const;

  bool hasFrequency() const;

  bool hasColors() const;

  virtual void setColor(QString color );
  virtual QString color();

  bool changed() const;
  void setChanged(bool changed);

  bool beingVisited() const;
  void setBeingVisited(bool beingVisited);

  bool visited() const;
  void setVisited(bool visited);

  bool isValid();

  void setLabel(QString label);
  QString getLabel();
protected:
  void setRotatable(bool rotatable);
  void setHasLabel(bool hasLabel);
  void setHasFrequency(bool hasFrequency);
  void setHasColors(bool hasColors);
  void setMinInputSz(int minInputSz);
  void setMinOutputSz(int minOutputSz);
  void setOutputsOnTop(bool outputsOnTop);
  void setMaxOutputSz(int maxOutputSz);
  void setMaxInputSz(int maxInputSz);
  void setTopPosition(int topPosition);
  void setBottomPosition(int bottomPosition);

//  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private:
  QGraphicsTextItem *label;
  int m_topPosition;
  int m_bottomPosition;
  int m_maxInputSz;
  int m_maxOutputSz;
  int m_minInputSz;
  int m_minOutputSz;
  bool m_outputsOnTop;
  bool m_rotatable;
  bool m_hasLabel;
  bool m_hasFrequency;
  bool m_hasColors;
  bool m_changed;
  bool m_beingVisited;
  bool m_visited;
  QVector<QNEPort*> m_inputs;
  QVector<QNEPort*> m_outputs;
};

#endif // GRAPHICELEMENT_H
