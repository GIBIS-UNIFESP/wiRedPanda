#ifndef LOGICAND_H
#define LOGICAND_H

#include "logicelement.h"

class LogicAnd : public LogicElement {
public:
  explicit LogicAnd( int inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

#endif // LOGICAND_H
