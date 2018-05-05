#include "logicsrflipflop.h"

LogicSRFlipFlop::LogicSRFlipFlop( ) :
  LogicElement( 5, 2 ),
  lastClk( false ) {
  setOutputValue( 0, false );
  setOutputValue( 1, true );
}

void LogicSRFlipFlop::_updateLogic( const std::vector< bool > &inputs ) {
  bool q0 = getOutputValue( 0 );
  bool q1 = getOutputValue( 1 );
  bool s = inputs[ 0 ];
  bool clk = inputs[ 1 ];
  bool r = inputs[ 2 ];
  bool prst = inputs[ 3 ];
  bool clr = inputs[ 4 ];
  if( clk && !lastClk ) {
    if( s && r ) {
      q0 = true;
      q1 = true;
    }
    else if( s != r ) {
      q0 = s;
      q1 = r;
    }
  }
  if( ( !prst ) || ( !clr ) ) {
    q0 = !prst;
    q1 = !clr;
  }
  lastClk = clk;

  setOutputValue( 0, q0 );
  setOutputValue( 1, q1 );
  /* Reference: https://pt.wikipedia.org/wiki/Flip-flop#Flip-flop_SR_Sincrono */
}
