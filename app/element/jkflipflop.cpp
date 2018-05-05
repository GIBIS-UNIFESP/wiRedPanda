#include "jkflipflop.h"

#include <QDebug>

JKFlipFlop::JKFlipFlop( QGraphicsItem *parent ) : GraphicElement( 5, 5, 2, 2, parent ) {
  setPixmap( ":/memory/JK-flipflop.png" );
  setRotatable( false );
  updatePorts( );
  lastClk = false;
  setPortName( "FlipFlop JK" );

  input( 0 )->setName( "J" );
  input( 1 )->setName( "Clock" );
  input( 2 )->setName( "K" );
  input( 3 )->setName( "~Preset" );
  input( 4 )->setName( "~Clear" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );
  output( 0 )->setDefaultValue( 0 );
  output( 1 )->setDefaultValue( 1 );

  input( 0 )->setRequired( false ); /* J */
  input( 2 )->setRequired( false ); /* K */
  input( 3 )->setRequired( false ); /* p */
  input( 4 )->setRequired( false ); /* c */
  input( 0 )->setDefaultValue( 1 );
  input( 2 )->setDefaultValue( 1 );
  input( 3 )->setDefaultValue( 1 );
  input( 4 )->setDefaultValue( 1 );
}

void JKFlipFlop::updatePorts( ) {
  input( 0 )->setPos( topPosition( ), 13 ); /* J */
  input( 1 )->setPos( topPosition( ), 29 ); /* Clk */
  input( 2 )->setPos( topPosition( ), 45 ); /* K */
  input( 3 )->setPos( 32, topPosition( ) ); /* Preset */
  input( 4 )->setPos( 32, bottomPosition( ) ); /* Clear */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void JKFlipFlop::updateLogic( ) {
  char q1 = output( 0 )->value( ); /* Q */
  char q2 = output( 1 )->value( ); /* ~Q */
  char j = input( 0 )->value( );
  char clk = input( 1 )->value( ); /* Current lock */
  char k = input( 2 )->value( );
  char prst = input( 3 )->value( );
  char clr = input( 4 )->value( );
  if( isValid( ) == false ) {
    q1 = -1;
    q2 = -1;
  }
  else {
    if( ( q1 == -1 ) || ( q2 == -1 ) ) {
      q1 = output( 0 )->defaultValue( );
      q2 = output( 1 )->defaultValue( );
    }
    if( ( clk == 1 ) && ( lastClk == 0 ) ) { /* If Clock up */
      if( ( lastJ == 1 ) && ( lastK == 1 ) ) { /* IF J=K */
        std::swap( q1, q2 );
      }
      else if( lastJ == 1 ) { /* If J == 1, set Q */
        q1 = 1;
        q2 = 0;
      }
      else if( lastK == 1 ) { /* If K == 1, reset Q */
        q1 = 0;
        q2 = 1;
      }
    }
    if( ( prst == 0 ) || ( clr == 0 ) ) {
      q1 = !prst;
      q2 = !clr;
    }
  }
  output( 0 )->setValue( q1 );
  output( 1 )->setValue( q2 );
  if( clk == -1 )
    lastClk = 0;
  else
    lastClk = clk;
  lastJ = j;
  lastK = k;
}
