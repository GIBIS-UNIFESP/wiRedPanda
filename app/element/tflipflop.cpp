#include "tflipflop.h"

TFlipFlop::TFlipFlop( QGraphicsItem *parent ) : GraphicElement( 4, 4, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/T-flipflop.png" ) );
  setRotatable( false );
  updatePorts( );
  lastClk = 0;
  setObjectName( "FlipFlop T" );

  inputs( ).at( 0 )->setName( "T" );
  inputs( ).at( 1 )->setName( "Clock" );
  inputs( ).at( 2 )->setName( "~Preset" );
  inputs( ).at( 3 )->setName( "~Clear" );
  outputs( ).at( 0 )->setName( "Q" );
  outputs( ).at( 1 )->setName( "~Q" );
  inputs( ).at( 0 )->setRequired( false );
  inputs( ).at( 2 )->setRequired( false );
  inputs( ).at( 3 )->setRequired( false );
  inputs( ).at( 2 )->setDefaultValue( 1 );
  inputs( ).at( 3 )->setDefaultValue( 1 );
}

TFlipFlop::~TFlipFlop( ) {

}

void TFlipFlop::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* T */
  inputs( ).at( 1 )->setPos( topPosition( ), 45 ); /* Clock */
  inputs( ).at( 2 )->setPos( 32, topPosition( ) ); /* Preset */
  inputs( ).at( 3 )->setPos( 32, bottomPosition( ) ); /* Clear */

  outputs( ).at( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  outputs( ).at( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void TFlipFlop::updateLogic( ) {
  char res1 = outputs( ).at( 0 )->value( ); /* Q */
  char res2 = outputs( ).at( 1 )->value( ); /* Q */
  if( !isValid( ) ) {
    res1 = -1;
    res2 = -1;
  }
  else {
    char T = inputs( ).at( 0 )->value( );
    char clk = inputs( ).at( 1 )->value( ); /* Current lock */
    char prst = inputs( ).at( 2 )->value( );
    char clr = inputs( ).at( 3 )->value( );
    if( ( clk == 1 ) && ( lastClk == 0 ) ) { /* If Clock up*/
      if( T == 1 ) { /* And T */
        res1 = !res1;
        res2 = !res1;
      }
    }
    if( ( prst == 0 ) || ( clr == 0 ) ) {
      res1 = !prst;
      res2 = !clr;
    }
    lastClk = clk;
  }
  outputs( ).at( 0 )->setValue( res1 );
  outputs( ).at( 1 )->setValue( res2 );
}

/* Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#T_flip-flop */
