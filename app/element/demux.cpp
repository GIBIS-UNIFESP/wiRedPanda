#include "demux.h"

Demux::Demux( QGraphicsItem *parent ) : GraphicElement( 2, 2, 2, 2, parent ) {
  pixmapSkinName.append( ":/basic/demux.png" );
  setPixmap( pixmapSkinName[ 0 ] );
  setRotatable( false );
  updatePorts( );
  setPortName( "DEMUX" );

  input( 0 )->setName( "in" );
  input( 1 )->setName( "S" );


  output( 0 )->setName( "out0" );
  output( 1 )->setName( "out1" );

}

void Demux::updatePorts( ) {
  input( 0 )->setPos( 16, 32 ); /* 0 */
  input( 1 )->setPos( 32, 58 ); /* S */
  output( 0 )->setPos( 48, 32 - 12 ); /* Out */
  output( 1 )->setPos( 48, 32 + 12 ); /* Out */
}

void Demux::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/demux.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
