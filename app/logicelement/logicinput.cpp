#include "logicinput.h"

LogicInput::LogicInput( bool defaultValue ) : LogicElement( 0, 1 ) {
  setOutputValue( 0, defaultValue );
}

void LogicInput::_updateLogic( const std::vector< bool > & ) {
  // Does nothing on update
}