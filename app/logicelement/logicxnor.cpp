#include "logicxnor.h"

LogicXnor::LogicXnor(int inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicXnor::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result ^= in;
  }
  setOutputValue( !result );
}
