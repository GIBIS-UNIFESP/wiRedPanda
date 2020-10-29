#ifndef LOGICMUX_H
#define LOGICMUX_H

#include "logicelement.h"

class LogicMux : public LogicElement {
public:
  explicit LogicMux( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};


#endif // LOGICMUX_H