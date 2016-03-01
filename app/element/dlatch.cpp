#include "dlatch.h"

DLatch::DLatch( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/D-latch.png" ) );
  setRotatable( false );
  updatePorts( );
  setObjectName( "D Latch" );
  inputs( ).at( 0 )->setName( "Data" );
  inputs( ).at( 1 )->setName( "Enable" );
  outputs( ).at( 0 )->setName( "Q" );
  outputs( ).at( 1 )->setName( "~Q" );
}

DLatch::~DLatch( ) {

}

void DLatch::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* Data */
  inputs( ).at( 1 )->setPos( topPosition( ), 45 ); /* Enable */

  outputs( ).at( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  outputs( ).at( 1 )->setPos( bottomPosition( ), 45 ); /* ~Q */
}

void DLatch::updateLogic( ) {
  char res1 = outputs( ).at( 0 )->value( ); /* Q */
  char res2 = outputs( ).at( 1 )->value( ); /* ~Q */
  char data = inputs( ).at( 0 )->value( );
  char enable = inputs( ).at( 1 )->value( );
  if( !isValid( ) ) {
    res1 = res2 = -1;
  }
  else {
    if( enable == 1 ) { /* If Enabled */
      res1 = data; /* Output = Data */
      res2 = !data; /* Output = Data */
    }
  }
  outputs( ).at( 0 )->setValue( res1 );
  outputs( ).at( 1 )->setValue( res2 );
}
