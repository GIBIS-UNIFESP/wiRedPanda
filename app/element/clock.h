#ifndef CLOCK_H
#define CLOCK_H

#include "graphicelement.h"
#include "globalproperties.h"
#include <QTime>

class Clock : public GraphicElement {
  int interval;
  int elapsed;
  bool on;
  double m_frequency;

public:
  explicit Clock(QGraphicsItem * parent);
  virtual ~Clock();
  static bool reset;
public slots:
  virtual ElementType elementType() {
    return ElementType::CLOCK;
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::INPUT );
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
  QString genericProperties();
private:
  void updateClock();

};

#endif // CLOCK_H
