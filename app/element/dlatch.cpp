#include "dlatch.h"

DLatch::DLatch( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  setPixmap( ":/memory/D-latch.png" );
  setRotatable( false );
  updatePorts( );
  setPortName( "D Latch" );
  input( 0 )->setName( "Data" );
  input( 1 )->setName( "Enable" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );

  output( 0 )->setDefaultValue( 0 );
  output( 1 )->setDefaultValue( 1 );
}

void DLatch::updatePorts( ) {
  input( 0 )->setPos( topPosition( ), 13 ); /* Data */
  input( 1 )->setPos( topPosition( ), 45 ); /* Enable */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void DLatch::updateLogic( ) {
  signed char res1 = output( 0 )->value( ); /* Q */
  signed char res2 = output( 1 )->value( ); /* ~Q */
  signed char data = input( 0 )->value( );
  signed char enable = input( 1 )->value( );
  if( !isValid( ) ) {
    res1 = res2 = -1;
  }
  else {
    if( res1 == -1 ) {
      res1 = 0;
      res2 = 1;
    }
    if( enable == 1 ) { /* If Enabled */
      res1 = data; /* Output = Data */
      res2 = !data; /* Output = Data */
    }
  }
  output( 0 )->setValue( res1 );
  output( 1 )->setValue( res2 );
}
