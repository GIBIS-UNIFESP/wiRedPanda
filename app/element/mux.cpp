#include "mux.h"

Mux::Mux( QGraphicsItem *parent ) : GraphicElement( 3, 3, 1, 1, parent ) {
  setPixmap( QPixmap( ":/basic/mux.png" ) );
  setRotatable( false );
  updatePorts( );
  setPortName( "MUX" );

  inputs( ).at( 0 )->setName( "0" );
  inputs( ).at( 1 )->setName( "1" );
  inputs( ).at( 2 )->setName( "S" );

}

void Mux::updatePorts( ) {
  inputs( ).at( 0 )->setPos( 16, 32 - 12 ); /* 0 */
  inputs( ).at( 1 )->setPos( 16, 32 + 12 ); /* 1 */
  inputs( ).at( 2 )->setPos( 32, 58 ); /* S */
  outputs( ).at( 0 )->setPos( 48, 32 ); /* Out */
}

void Mux::updateLogic( ) {
  char data1 = inputs( ).at( 0 )->value( );
  char data2 = inputs( ).at( 1 )->value( );
  char choice = inputs( ).at( 2 )->value( );
  char res = -1;
  if( isValid( ) ) {
    if( choice == 0 ) {
      res = data1;
    }
    else {
      res = data2;
    }
  }
  outputs( ).at( 0 )->setValue( res );
}
