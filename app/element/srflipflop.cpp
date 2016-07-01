#include "srflipflop.h"

SRFlipFlop::SRFlipFlop( QGraphicsItem *parent ) : GraphicElement( 3, 3, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/SR-flipflop.png" ) );
  setRotatable( false );
  updatePorts( );
  lastClk = false;
  setPortName( "FlipFlop SR" );

  inputs( ).at( 0 )->setName( "S" );
  inputs( ).at( 1 )->setName( "Clock" );
  inputs( ).at( 2 )->setName( "R" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );
  inputs( ).at( 0 )->setRequired( false );
  inputs( ).at( 2 )->setRequired( false );
}

SRFlipFlop::~SRFlipFlop( ) {

}

void SRFlipFlop::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* S */
  inputs( ).at( 1 )->setPos( topPosition( ), 29 ); /* Clk */
  inputs( ).at( 2 )->setPos( topPosition( ), 45 ); /* R */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void SRFlipFlop::updateLogic( ) {
  //TODO: Add set and preset options.
  char q1 = output( 0 )->value( ); /* Q */
  char q2 = output( 1 )->value( ); /* ~Q */
  if( !isValid( ) ) {
    q1 = -1;
    q2 = -1;
  }
  else {
    if( q1 == -1 ) {
      q1 = 0;
      q2 = 1;
    }
    char s = inputs( ).at( 0 )->value( );
    char clk = inputs( ).at( 1 )->value( );
    char r = inputs( ).at( 2 )->value( );
    if( ( clk == 1 ) && ( lastClk == 0 ) ) { /* If Clock up */
      if( s && r ) { /* Not permitted */
        q1 = 1;
        q2 = 1;
      }
      else if( s != r ) {
        q1 = s;
        q2 = r;
      }
    }
    lastClk = clk;
  }
  output( 0 )->setValue( q1 );
  output( 1 )->setValue( q2 );
  /* Reference: https://pt.wikipedia.org/wiki/Flip-flop#Flip-flop_SR_Sincrono */
}
