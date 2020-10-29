#include "xor.h"

Xor::Xor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/xor.png" );
  updatePorts( );
  setPortName( "XOR" );
}

void Xor::updateLogic( ) {
  signed char res = false;
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    res = 0;
    for( QNEPort *input : qAsConst(m_inputs) ) {
      res = res ^ input->value( );
    }
  }
  m_outputs.first( )->setValue( res );
}
