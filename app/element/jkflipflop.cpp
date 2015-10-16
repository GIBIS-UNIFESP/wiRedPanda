#include "jkflipflop.h"

#include <QDebug>

JKFlipFlop::JKFlipFlop( QGraphicsItem *parent ) : GraphicElement( 5, 5, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/JK-flipflop.png" ) );
  setRotatable( false );
  updatePorts( );
  lastClk = false;
  setObjectName( "FlipFlop JK" );

  inputs( ).at( 0 )->setName( "J" );
  inputs( ).at( 1 )->setName( "Clock" );
  inputs( ).at( 2 )->setName( "K" );
  inputs( ).at( 3 )->setName( "~Preset" );
  inputs( ).at( 4 )->setName( "~Clear" );
  outputs( ).at( 0 )->setName( "Q" );
  outputs( ).at( 1 )->setName( "~Q" );
  inputs( ).at( 0 )->setRequired( false );
  inputs( ).at( 2 )->setRequired( false );
  inputs( ).at( 3 )->setRequired( false );
  inputs( ).at( 4 )->setRequired( false );
  inputs( ).at( 3 )->setDefaultValue( 1 );
  inputs( ).at( 4 )->setDefaultValue( 1 );
}

JKFlipFlop::~JKFlipFlop( ) {

}

void JKFlipFlop::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* J */
  inputs( ).at( 1 )->setPos( topPosition( ), 29 ); /* Clk */
  inputs( ).at( 2 )->setPos( topPosition( ), 45 ); /* K */
  inputs( ).at( 3 )->setPos( 32, topPosition( ) ); /* Preset */
  inputs( ).at( 4 )->setPos( 32, bottomPosition( ) ); /* Clear */

  outputs( ).at( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  outputs( ).at( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void JKFlipFlop::updateLogic( ) {
  char res1 = outputs( ).at( 0 )->value( ); /* Q */
  char res2 = outputs( ).at( 1 )->value( ); /* ~Q */
  if( !isValid( ) ) {
    res1 = -1;
    res2 = -1;
  }
  else {
    char j = inputs( ).at( 0 )->value( );
    bool clk = inputs( ).at( 1 )->value( ); /* Current lock */
    char k = inputs( ).at( 2 )->value( );
    char prst = inputs( ).at( 3 )->value( );
    char clr = inputs( ).at( 4 )->value( );
    if( ( clk == true ) && ( lastClk == false ) ) { /* If Clock up */
      res1 = ( j && res2 ) || ( !k && res1 );
      res2 = !res1;
    }
    if( ( prst == false ) || ( clr == false ) ) {
      res1 = !prst;
      res2 = !clr;
    }
    lastClk = clk;
  }
  outputs( ).at( 0 )->setValue( res1 );
  outputs( ).at( 1 )->setValue( res2 );
}
