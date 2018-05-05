#include "logicoutput.h"

LogicOutput::LogicOutput( int inputSz ) : LogicElement( inputSz, inputSz ) {

}

void LogicOutput::_updateLogic( const std::vector< bool > &inputs ) {
  for( int idx = 0; idx < inputs.size( ); ++idx ) {
    setOutputValue( idx, inputs[ idx ] );
  }
}
