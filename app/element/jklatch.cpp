#include "jklatch.h"

JKLatch::JKLatch( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  setPixmap( ":/memory/JK-latch.png" );
  setRotatable( false );
  updatePorts( );
  setPortName( "JK Latch" );
}

void JKLatch::updatePorts( ) {
  input( 0 )->setPos( topPosition( ), 13 );
  input( 1 )->setPos( topPosition( ), 45 );

  output( 0 )->setPos( bottomPosition( ), 15 );
  output( 1 )->setPos( bottomPosition( ), 45 );
}

void JKLatch::updateLogic( ) {
  char res = m_outputs.first( )->value( );
  char j = input( 0 )->value( );
  char k = input( 1 )->value( );
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    if( res == -1 ) {
      res = 0;
    }
    if( ( j == 1 ) && ( k == 1 ) ) { /* IF J=K */
      res = !res;
    }
    else if( j != k ) { /* J */
      res = j;
    } /* else nothing happens */
  }
  m_outputs.first( )->setValue( res );
  m_outputs.last( )->setValue( !res );
}

/* Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#Gated_latches_and_conditional_transparency */