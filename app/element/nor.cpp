#include "nor.h"

Nor::Nor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/nor.png" );
  updatePorts( );
  setPortName( "NOR" );
}
