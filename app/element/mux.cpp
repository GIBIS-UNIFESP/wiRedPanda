#include "mux.h"

Mux::Mux( QGraphicsItem *parent ) : GraphicElement( 3, 3, 1, 1, parent ) {
  setPixmap( ":/basic/mux.png" );
  setRotatable( false );
  updatePorts( );
  setPortName( "MUX" );

  input( 0 )->setName( "0" );
  input( 1 )->setName( "1" );
  input( 2 )->setName( "S" );

}

void Mux::updatePorts( ) {
  input( 0 )->setPos( 16, 32 - 12 ); /* 0 */
  input( 1 )->setPos( 16, 32 + 12 ); /* 1 */
  input( 2 )->setPos( 32, 58 ); /* S */
  output( 0 )->setPos( 48, 32 ); /* Out */
}
