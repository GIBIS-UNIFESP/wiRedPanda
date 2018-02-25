#include "logicoutput.h"

LogicOutput::LogicOutput( size_t inputSz ) : LogicElement( inputSz, inputSz ) {

}

void LogicOutput::_updateLogic( const std::vector< bool > &inputs ) {
  for( size_t idx = 0; idx < inputs.size( ); ++idx ) {
    setOutputValue( idx, inputs[ idx ] );
  }
}
