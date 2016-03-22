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
  outputs( ).at( 0 )->setName( "Q" );
  outputs( ).at( 1 )->setName( "~Q" );
  inputs( ).at( 0 )->setRequired( false );
  inputs( ).at( 2 )->setRequired( false );
}

SRFlipFlop::~SRFlipFlop( ) {

}

void SRFlipFlop::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* S */
  inputs( ).at( 1 )->setPos( topPosition( ), 29 ); /* Clk */
  inputs( ).at( 2 )->setPos( topPosition( ), 45 ); /* R */

  outputs( ).at( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  outputs( ).at( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void SRFlipFlop::updateLogic( ) {
  char res1 = outputs( ).at( 0 )->value( ); /* Q */
  char res2 = outputs( ).at( 1 )->value( ); /* ~Q */
  if( !isValid( ) ) {
    res1 = -1;
    res2 = -1;
  }
  else {
    if( res1 == -1 ) {
      res1 = 0;
      res2 = 0;
    }
    char s = inputs( ).at( 0 )->value( );
    char clk = inputs( ).at( 1 )->value( );
    char r = inputs( ).at( 2 )->value( );
    if( ( clk == 1 ) && ( lastClk == 0 ) ) { /* If Clock up */
      if( s && r ) { /* Not permitted */
        res1 = 1;
        res2 = 1;
      }
      else if( s != r ) {
        res1 = s;
        res2 = r;
      }
    }
    lastClk = clk;
  }
  outputs( ).at( 0 )->setValue( res1 );
  outputs( ).at( 1 )->setValue( res2 );
  /* Reference: https://pt.wikipedia.org/wiki/Flip-flop#Flip-flop_SR_Sincrono */
}
