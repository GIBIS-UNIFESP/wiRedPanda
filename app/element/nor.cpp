#include "nor.h"

Nor::Nor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  pixmapSkinName.append( ":/basic/nor.png" );
  setOutputsOnTop( true );
  setPixmap( pixmapSkinName[ 0 ] );
  setCanChangeSkin( true );
  updatePorts( );
  setPortName( "NOR" );
}

void Nor::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/nor.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
