#include "demux.h"

Demux::Demux( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  setPixmap( QPixmap( ":/basic/demux.png" ) );
  setRotatable( false );
  updatePorts( );
  setPortName( "DEMUX" );

  inputs( ).at( 0 )->setName( "in" );
  inputs( ).at( 1 )->setName( "S" );


  output( 0 )->setName( "out0" );
  output( 1 )->setName( "out1" );

}

void Demux::updatePorts( ) {
  inputs( ).at( 0 )->setPos( 16, 32 ); /* 0 */
  inputs( ).at( 1 )->setPos( 32, 58 ); /* S */
  output( 0 )->setPos( 48, 32 - 12 ); /* Out */
  output( 1 )->setPos( 48, 32 + 12 ); /* Out */
}

void Demux::updateLogic( ) {
  char in = inputs( ).at( 0 )->value( );
  char choice = inputs( ).at( 1 )->value( );

  char out0 = -1;
  char out1 = -1;
  if( isValid( ) ) {
    if( choice == 0 ) {
      out0 = in;
      out1 = false;
    }
    else {
      out0 = false;
      out1 = in;
    }
  }
  output( 0 )->setValue( out0 );
  output( 1 )->setValue( out1 );
}
