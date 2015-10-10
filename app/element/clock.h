#ifndef CLOCK_H
#define CLOCK_H

#include "graphicelement.h"

#include <QTime>

class Clock : public GraphicElement {
  int interval;
  int elapsed;
  bool on;
  double m_frequency;

public:
  const static int simulationInterval = 10;
  explicit Clock(QGraphicsItem * parent);
  ~Clock();
  static bool reset;
public slots:
  virtual ElementType elementType() {
    return ElementType::CLOCK;
  }
//  void updateClock();

  // GraphicElement interface
public:
  void save(QDataStream & ds);
  void load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap, double version);
  float frequency();
  void setFrequency(float freq);
  void updateLogic();
  void resetClock();
private:
  constexpr static float frequencyMultiplier = 1000.0/simulationInterval;
  void updateClock();
};

#endif // CLOCK_H
