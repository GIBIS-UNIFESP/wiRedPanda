#include "logicmux.h"

LogicMux::LogicMux( ) : LogicElement( 3, 1 ) {

}

void LogicMux::_updateLogic( const std::vector< bool > &inputs ) {
  bool data1 = inputs[ 0 ];
  bool data2 = inputs[ 1 ];
  bool choice = inputs[ 2 ];
  if( choice == false ) {
    setOutputValue( data1 );
  }
  else {
    setOutputValue( data2 );
  }
}
