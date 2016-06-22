#include "jkflipflop.h"

#include <QDebug>

JKFlipFlop::JKFlipFlop( QGraphicsItem *parent ) : GraphicElement( 5, 5, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/JK-flipflop.png" ) );
  setRotatable( false );
  updatePorts( );
  lastClk = false;
  setPortName( "FlipFlop JK" );

  inputs( ).at( 0 )->setName( "J" );
  inputs( ).at( 1 )->setName( "Clock" );
  inputs( ).at( 2 )->setName( "K" );
  inputs( ).at( 3 )->setName( "~Preset" );
  inputs( ).at( 4 )->setName( "~Clear" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );
  inputs( ).at( 0 )->setRequired( false );
  inputs( ).at( 2 )->setRequired( false );
  inputs( ).at( 3 )->setRequired( false );
  inputs( ).at( 4 )->setRequired( false );
  inputs( ).at( 3 )->setDefaultValue( 1 );
  inputs( ).at( 4 )->setDefaultValue( 1 );
}

void JKFlipFlop::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* J */
  inputs( ).at( 1 )->setPos( topPosition( ), 29 ); /* Clk */
  inputs( ).at( 2 )->setPos( topPosition( ), 45 ); /* K */
  inputs( ).at( 3 )->setPos( 32, topPosition( ) ); /* Preset */
  inputs( ).at( 4 )->setPos( 32, bottomPosition( ) ); /* Clear */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void JKFlipFlop::updateLogic( ) {
  char q1 = output( 0 )->value( ); /* Q */
  char q2 = output( 1 )->value( ); /* ~Q */
  if( isValid( ) == false ) {
    q1 = -1;
    q2 = -1;
  }
  else {
    if( q1 == -1 ) {
      q1 = 0;
      q2 = 0;
    }
    char j = inputs( ).at( 0 )->value( );
    char clk = inputs( ).at( 1 )->value( ); /* Current lock */
    char k = inputs( ).at( 2 )->value( );
    char prst = inputs( ).at( 3 )->value( );
    char clr = inputs( ).at( 4 )->value( );
    if( ( clk == 1 ) && ( lastClk == 0 ) ) { /* If Clock up */
      if( ( j == 1 ) && ( k == 1 ) ) { /* IF J=K */
        std::swap( q1, q2 );
      }
      else if( j == 1 ) { /* If J == 1, set Q */
        q1 = 1;
        q2 = 0;
      }
      else if( k == 1 ) { /* If K == 1, reset Q */
        q1 = 0;
        q2 = 1;
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
}
