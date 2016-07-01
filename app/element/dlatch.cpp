#include "dlatch.h"

DLatch::DLatch( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/D-latch.png" ) );
  setRotatable( false );
  updatePorts( );
  setPortName( "D Latch" );
  inputs( ).at( 0 )->setName( "Data" );
  inputs( ).at( 1 )->setName( "Enable" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );
}

DLatch::~DLatch( ) {

}

void DLatch::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* Data */
  inputs( ).at( 1 )->setPos( topPosition( ), 45 ); /* Enable */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void DLatch::updateLogic( ) {
  char res1 = output( 0 )->value( ); /* Q */
  char res2 = output( 1 )->value( ); /* ~Q */
  char data = inputs( ).at( 0 )->value( );
  char enable = inputs( ).at( 1 )->value( );
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
