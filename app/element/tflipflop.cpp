#include "tflipflop.h"

TFlipFlop::TFlipFlop( QGraphicsItem *parent ) : GraphicElement( 4, 4, 2, 2, parent ) {
  setPixmap( ":/memory/T-flipflop.png" );
  setRotatable( false );
  updatePorts( );
  lastClk = false;
/*  lastT = 0; */
  setPortName( "FlipFlop T" );

  input( 0 )->setName( "T" );
  input( 1 )->setName( "Clock" );
  input( 2 )->setName( "~Preset" );
  input( 3 )->setName( "~Clear" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );

  output( 0 )->setDefaultValue( 0 );
  output( 1 )->setDefaultValue( 1 );

  input( 0 )->setRequired( false );
  input( 2 )->setRequired( false );
  input( 3 )->setRequired( false );
  input( 2 )->setDefaultValue( 1 );
  input( 3 )->setDefaultValue( 1 );
}

TFlipFlop::~TFlipFlop( ) {

}

void TFlipFlop::updatePorts( ) {
  input( 0 )->setPos( topPosition( ), 13 ); /* T */
  input( 1 )->setPos( topPosition( ), 45 ); /* Clock */
  input( 2 )->setPos( 32, topPosition( ) ); /* Preset */
  input( 3 )->setPos( 32, bottomPosition( ) ); /* Clear */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void TFlipFlop::updateLogic( ) {
  char q1 = output( 0 )->value( ); /* Q */
  char q2 = output( 1 )->value( ); /* Q */
  if( !isValid( ) ) {
    q1 = -1;
    q2 = -1;
  }
  else {
    if( ( q1 == -1 ) || ( q2 == -1 ) ) {
      q1 = output( 0 )->defaultValue( );
      q2 = output( 1 )->defaultValue( );
    }
    char T = input( 0 )->value( );
    char clk = input( 1 )->value( ); /* Current lock */
    char prst = input( 2 )->value( );
    char clr = input( 3 )->value( );
    if( ( clk == 0 ) && ( lastClk == 1 ) ) { /* If Clock up*/
      if( T == 1 ) { /* And T */
        q1 = !q1;
        q2 = !q1;
      }
    }
    if( ( prst == 0 ) || ( clr == 0 ) ) {
      q1 = !prst;
      q2 = !clr;
    }
    lastClk = clk;
/*    lastT = T; */
  }
  output( 0 )->setValue( q1 );
  output( 1 )->setValue( q2 );
}

/* Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#T_flip-flop */
