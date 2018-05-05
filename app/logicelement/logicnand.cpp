#include "logicnand.h"

LogicNand::LogicNand(int inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicNand::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = true;
  for( bool in : inputs ) {
    result &= in;
  }
  setOutputValue( !result );
}
