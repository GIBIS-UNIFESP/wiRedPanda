#include "not.h"

Not::Not( QGraphicsItem *parent ) : GraphicElement( 1, 1, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/not.png" );
  updatePorts( );
  setPortName( "NOT" );
}

void Not::updateLogic( ) {
  char res = !inputs( ).first( )->value( );
  if( !isValid( ) ) {
    res = -1;
  }
  output( )->setValue( res );
}
