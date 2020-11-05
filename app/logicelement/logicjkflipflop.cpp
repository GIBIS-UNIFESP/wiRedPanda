#include "logicjkflipflop.h"

LogicJKFlipFlop::LogicJKFlipFlop( ) :
  LogicElement( 5, 2 ),
  lastClk( false ) {
  setOutputValue( 0, false );
  setOutputValue( 1, true );
}

void LogicJKFlipFlop::_updateLogic( const std::vector< bool > &inputs ) {
  bool q0 = getOutputValue( 0 );
  bool q1 = getOutputValue( 1 );
  bool j = inputs[ 0 ];
  bool clk = inputs[ 1 ];
  bool k = inputs[ 2 ];
  bool prst = inputs[ 3 ];
  bool clr = inputs[ 4 ];
  if( clk && !lastClk ) {
    if( lastJ && lastK ) {
      std::swap( q0, q1 );
    }
    else if( lastJ ) {
      q0 = true;
      q1 = false;
    }
    else if( lastK ) {
      q0 = false;
      q1 = true;
    }
  }
  if( ( !prst ) || ( !clr ) ) {
    q0 = !prst;
    q1 = !clr;
  }
  lastClk = clk;
  lastK = k;
  lastJ = j;

  setOutputValue( 0, q0 );
  setOutputValue( 1, q1 );
}