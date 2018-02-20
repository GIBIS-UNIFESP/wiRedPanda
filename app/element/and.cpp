#include "and.h"

And::And( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/and.png" );
  updatePorts( );
  setPortName( "AND" );
  setToolTip( "AND" );
}
