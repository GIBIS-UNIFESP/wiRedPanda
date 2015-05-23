#ifndef CLOCK_H
#define CLOCK_H

#include "graphicelement.h"

#include <QTimer>

class Clock : public GraphicElement {
  QTimer timer;
  bool on;
public:
  explicit Clock(QGraphicsItem * parent);
  ~Clock();
public slots:
  virtual ElementType elementType() {
    return ElementType::CLOCK;
  }
  void updateClock();

  // GraphicElement interface
public:
  void save(QDataStream & ds);
  void load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap);
  float frequency();
  void setFrequency(float freq);

private:
  double m_frequency;

};

#endif // CLOCK_H
