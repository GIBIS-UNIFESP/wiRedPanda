#include "xor.h"

Xor::Xor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/xor.png" );
  updatePorts( );
  setPortName( "XOR" );
}
