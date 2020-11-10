#include "xor.h"

Xor::Xor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  pixmapSkinName.append( ":/basic/xor.png" );
  setOutputsOnTop( true );
  setCanChangeSkin( true );
  setPixmap( pixmapSkinName[ 0 ] );
  updatePorts( );
  setPortName( "XOR" );
}

void Xor::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/xor.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
