#ifndef LOGICSRFLIPFLOP_H
#define LOGICSRFLIPFLOP_H

#include "logicelement.h"

class LogicSRFlipFlop : public LogicElement {
public:
  explicit LogicSRFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
};

#endif // LOGICSRFLIPFLOP_H
