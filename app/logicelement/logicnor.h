#ifndef LOGICNOR_H
#define LOGICNOR_H

#include "logicelement.h"

class LogicNor : public LogicElement {
public:
  explicit LogicNor( int inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};
#endif // LOGICNOR_H
