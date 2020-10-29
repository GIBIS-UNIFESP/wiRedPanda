#include "nor.h"

Nor::Nor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/nor.png" );
  updatePorts( );
  setPortName( "NOR" );
}

void Nor::updateLogic( ) {
  char res = true;
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    for( QNEPort *input: qAsConst(m_inputs) ) {
      if( input->value( ) == true ) {
        res = false;
        break;
      }
    }
  }
  m_outputs.first( )->setValue( res );
}