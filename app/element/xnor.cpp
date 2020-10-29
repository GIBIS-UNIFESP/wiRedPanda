#include "xnor.h"

Xnor::Xnor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/xnor.png" );
  updatePorts( );
  setPortName( "XNOR" );
}

void Xnor::updateLogic( ) {
  signed char res = false;
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    for( QNEPort *input : qAsConst(m_inputs) ) {
      res = res ^ input->value( );
    }
    res = !res;
  }
  m_outputs.first( )->setValue( res );
}
