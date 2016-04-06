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
  char res = false;
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    for( QNEPort *input : inputs( ) ) {
      res = res ^ input->value( );
    }
  }
  outputs( ).first( )->setValue( !res );
}
