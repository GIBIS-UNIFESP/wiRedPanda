#include "logicor.h"

LogicOr::LogicOr( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicOr::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result |= in;
  }
  setOutputValue( result );
}