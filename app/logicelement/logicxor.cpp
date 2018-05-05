#include "logicxor.h"

LogicXor::LogicXor(int inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicXor::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result ^= in;
  }
  setOutputValue( result );
}
