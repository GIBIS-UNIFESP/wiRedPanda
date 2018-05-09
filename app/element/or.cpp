#include "or.h"

Or::Or( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/or.png" );
  updatePorts( );
  setPortName( "OR" );
}
