#include "nand.h"

Nand::Nand( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/nand.png" );
  updatePorts( );
  setPortName( "NAND" );
}
