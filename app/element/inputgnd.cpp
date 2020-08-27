#include "inputgnd.h"

InputGnd::InputGnd( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  setOutputsOnTop( false );
  setPixmap( ":/input/0.png" );
  setRotatable( false );
  setPortName( "GND" );
  outputs( ).first( )->setValue( false );
}

