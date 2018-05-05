#ifndef LOGICJKFLIPFLOP_H
#define LOGICJKFLIPFLOP_H

#include "logicelement.h"

class LogicJKFlipFlop : public LogicElement {
public:
  explicit LogicJKFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
  bool lastJ;
  bool lastK;
};


#endif // LOGICJKFLIPFLOP_H
