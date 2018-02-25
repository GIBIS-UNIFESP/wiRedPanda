#include "logicnot.h"

LogicNot::LogicNot( ) : LogicElement( 1, 1 ) {

}

void LogicNot::_updateLogic( const std::vector< bool > &inputs ) {
  setOutputValue( !inputs[ 0 ] );
}
