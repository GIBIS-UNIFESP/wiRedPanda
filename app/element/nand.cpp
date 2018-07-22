#include "nand.h"

Nand::Nand( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  setOutputsOnTop( true );
  setPixmap( ":/basic/nand.png" );
  updatePorts( );
  setPortName( "NAND" );
}

void Nand::updateLogic( ) {
  char res = false;
  if( !isValid( ) ) {
    res = -1;
  }
  else {
    for( QNEPort *input: inputs( ) ) {
      if( input->value( ) == false ) {
        res = true;
        break;
      }
    }
  }
  outputs( ).first( )->setValue( res );
}
