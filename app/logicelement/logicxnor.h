#ifndef LOGICXNOR_H
#define LOGICXNOR_H

#include <logicelement.h>



class LogicXnor : public LogicElement {
public:
  explicit LogicXnor( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};


#endif // LOGICXNOR_H
