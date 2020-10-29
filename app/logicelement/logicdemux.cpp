#include "logicdemux.h"

LogicDemux::LogicDemux( ) : LogicElement( 2, 2 ) {

}

void LogicDemux::_updateLogic( const std::vector< bool > &inputs ) {
  bool data = inputs[ 0 ];
  bool choice = inputs[ 1 ];

  bool out0 = false;
  bool out1 = false;
  if( choice == false ) {
    out0 = data;
  }
  else {
    out1 = data;
  }
  setOutputValue( 0, out0 );
  setOutputValue( 1, out1 );
}