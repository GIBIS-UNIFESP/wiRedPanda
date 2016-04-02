#include "tlatch.h"

TLatch::TLatch( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  setPixmap( QPixmap( ":/memory/T-latch.png" ) );
  setRotatable( false );
  updatePorts( );
  setPortName( "T Latch" );
  inputs( ).at( 0 )->setName( "Toggle" );
  inputs( ).at( 1 )->setName( "Enable" );
  output( 0 )->setName( "Q" );
  output( 1 )->setName( "~Q" );
}

TLatch::~TLatch( ) {

}

void TLatch::updatePorts( ) {
  inputs( ).at( 0 )->setPos( topPosition( ), 13 ); /* T */
  inputs( ).at( 1 )->setPos( topPosition( ), 45 ); /* Clock */

  output( 0 )->setPos( bottomPosition( ), 15 ); /* Q */
  output( 1 )->setPos( bottomPosition( ), 45 ); /* !Q */
}

void TLatch::updateLogic( ) {
  char res = output( 0 )->value( );
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    if( res == -1 ) {
      res = 0;
    }
    char toggle = inputs( ).at( 0 )->value( );
    char enable = inputs( ).at( 1 )->value( );
    if( enable == 1 ) { /* If Enable */
      if( toggle == 1 ) { /* And T */
        res = !res;
      }
    }
  }
  output( 0 )->setValue( res );
  output( 1 )->setValue( !res );
}
