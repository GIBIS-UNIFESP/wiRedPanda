#ifndef LOGICNAND_H
#define LOGICNAND_H

#include <logicelement.h>


class LogicNand : public LogicElement {
public:
  explicit LogicNand( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};


#endif // LOGICNAND_H
