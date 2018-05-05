#ifndef LOGICTFLIPFLOP_H
#define LOGICTFLIPFLOP_H

#include "logicelement.h"

class LogicTFlipFlop : public LogicElement {
public:
  explicit LogicTFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
  bool lastValue;
};

#endif // LOGICTFLIPFLOP_H
