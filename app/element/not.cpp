#include "not.h"

Not::Not( QGraphicsItem *parent ) : GraphicElement( 1, 1, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/not.png" );
  updatePorts( );
  setPortName( "NOT" );
}
