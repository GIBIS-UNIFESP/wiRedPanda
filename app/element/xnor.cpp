#include "xnor.h"

Xnor::Xnor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( QPixmap( ":/basic/xnor.png" ) );
  updatePorts( );
  setPortName( "XNOR" );
}

Xnor::~Xnor( ) {

}

void Xnor::updateLogic( ) {
  char res = true;
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    res = 0;
    for( QNEPort *input : inputs( ) ) {
      if( input->value( ) != true ) {
        res++;
      }
    }
    if( ( res % 2 == 0 ) || ( res == 0 ) ) {
      res = true;
    }
    else {
      res = false;
    }
  }
  outputs( ).first( )->setValue( res );
}
