#ifndef LOGICINPUT_H
#define LOGICINPUT_H

#include "logicelement.h"

class LogicInput : public LogicElement {
public:

  explicit LogicInput( bool defaultValue = false );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > & );
};


#endif // LOGICINPUT_H
