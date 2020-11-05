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

