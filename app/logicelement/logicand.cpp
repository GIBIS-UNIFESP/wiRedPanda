#include "logicand.h"

LogicAnd::LogicAnd( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicAnd::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = true;
  for( bool in : inputs ) {
    result &= in;
  }
  setOutputValue( result );
}
