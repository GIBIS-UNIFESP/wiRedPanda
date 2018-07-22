#ifndef LOGICNODE_H
#define LOGICNODE_H

#include "logicelement.h"

class LogicNode : public LogicElement {
public:

  explicit LogicNode( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > & );
};


#endif // LOGICNODE_H
