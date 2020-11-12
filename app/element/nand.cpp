#include "nand.h"

Nand::Nand( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  pixmapSkinName.append( ":/basic/nand.png" );
  setOutputsOnTop( true );
  setPixmap( pixmapSkinName[ 0 ] );
  setCanChangeSkin( true );
  updatePorts( );
  setCanChangeSkin( true );
  setPortName( "NAND" );
}

void Nand::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/nand.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
