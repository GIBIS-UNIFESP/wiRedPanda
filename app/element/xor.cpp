#include "xor.h"

Xor::Xor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( QPixmap( ":/basic/xor.png" ) );
  updatePorts( );
  setPortName( "XOR" );
}

Xor::~Xor( ) {

}

void Xor::updateLogic( ) {
  char res = false;
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    res = 0;
    for( QNEPort *input : inputs( ) ) {
      res = res ^ input->value( );
    }
  }
  outputs( ).first( )->setValue( res );
}
