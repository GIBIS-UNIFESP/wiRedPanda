#include "logicnor.h"

LogicNor::LogicNor(int inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicNor::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result |= in;
  }
  setOutputValue( !result );
}
