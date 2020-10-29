#include "srflipflop.h"

SRFlipFlop::SRFlipFlop( QGraphicsItem *parent ) : GraphicElement( 5, 5, 2, 2, parent ) {
  setPixmap( ":/memory/SR-flipflop.png" );
  setRotatable( false );
  updatePorts( );
  lastClk = false;
  setPortName( "FlipFlop SR" );

  input( 0 )->setName( "S" );
  input( 1 )->setName( "Clock" );
  input( 2 )->setName( "R" );
  input( 3 )->setName( "~Preset" );
  input( 4 )->setName( "~Clear" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );

  output( 0 )->setDefaultValue( 0 );
  output( 1 )->setDefaultValue( 1 );

  input( 0 )->setRequired( false ); /* S */
  input( 2 )->setRequired( false ); /* R */
  input( 3 )->setRequired( false ); /* p */
  input( 4 )->setRequired( false ); /* c */
  input( 3 )->setDefaultValue( 1 );
  input( 4 )->setDefaultValue( 1 );
}

void SRFlipFlop::updatePorts( ) {
  input( 0 )->setPos( topPosition( ), 13 ); /* S */
  input( 1 )->setPos( topPosition( ), 29 ); /* Clk */
  input( 2 )->setPos( topPosition( ), 45 ); /* R */
  input( 3 )->setPos( 32, topPosition( ) ); /* Preset */
  input( 4 )->setPos( 32, bottomPosition( ) ); /* Clear */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void SRFlipFlop::updateLogic( ) {
  char q1 = output( 0 )->value( ); /* Q */
  char q2 = output( 1 )->value( ); /* ~Q */
  if( !isValid( ) ) {
    q1 = -1;
    q2 = -1;
  }
  else {
    if( ( q1 == -1 ) || ( q2 == -1 ) ) {
      q1 = output( 0 )->defaultValue( );
      q2 = output( 1 )->defaultValue( );
    }
    char s = input( 0 )->value( );
    char clk = input( 1 )->value( );
    char r = input( 2 )->value( );
    char prst = input( 3 )->value( );
    char clr = input( 4 )->value( );
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
    if( ( prst == 0 ) || ( clr == 0 ) ) {
      q1 = !prst;
      q2 = !clr;
    }
    lastClk = clk;
  }
  output( 0 )->setValue( q1 );
  output( 1 )->setValue( q2 );
  /* Reference: https://pt.wikipedia.org/wiki/Flip-flop#Flip-flop_SR_Sincrono */
}