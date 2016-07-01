#include "dflipflop.h"

DFlipFlop::DFlipFlop( QGraphicsItem *parent ) : GraphicElement( 4, 4, 2, 2, parent ) {
  setPixmap( ":/memory/D-flipflop.png" );
  setRotatable( false );
  updatePorts( );
  setPortName( "FlipFlop D" );
  lastClk = false;
  inputs( ).at( 0 )->setName( "Data" );
  inputs( ).at( 1 )->setName( "Clock" );
  inputs( ).at( 2 )->setName( "~Preset" );
  inputs( ).at( 3 )->setName( "~Clear" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );
  inputs( ).at( 2 )->setRequired( false );
  inputs( ).at( 3 )->setRequired( false );
  inputs( ).at( 2 )->setDefaultValue( 1 );
  inputs( ).at( 3 )->setDefaultValue( 1 );
  lastValue = false;
  lastClk = false;
}

DFlipFlop::~DFlipFlop( ) {

}

void DFlipFlop::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* Data */
  inputs( ).at( 1 )->setPos( topPosition( ), 45 ); /* Clock */
  inputs( ).at( 2 )->setPos( 32, topPosition( ) ); /* Preset */
  inputs( ).at( 3 )->setPos( 32, bottomPosition( ) ); /* Clear */
  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void DFlipFlop::updateLogic( ) {
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
    char data = inputs( ).at( 0 )->value( );
    bool clk = inputs( ).at( 1 )->value( ); /* Current lock */
    char prst = inputs( ).at( 2 )->value( );
    char clr = inputs( ).at( 3 )->value( );
    if( ( clk == true ) && ( lastClk == false ) ) { /* If Clock up */
      q1 = lastValue; /* Output = Data */
      q2 = !lastValue;
    }
    if( ( prst == 0 ) || ( clr == 0 ) ) {
      q1 = !prst;
      q2 = !clr;
    }
    lastClk = clk;
    lastValue = data;
  }
  outputs( ).first( )->setValue( q1 );
  outputs( ).last( )->setValue( q2 );
}
