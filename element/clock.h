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
  void updateClock();
};

#endif // CLOCK_H
