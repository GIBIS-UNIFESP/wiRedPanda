#ifndef LOGICDFLIPFLOP_H
#define LOGICDFLIPFLOP_H

#include "logicelement.h"

class LogicDFlipFlop : public LogicElement {
public:
  explicit LogicDFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
  bool lastValue;
};


#endif // LOGICDFLIPFLOP_H