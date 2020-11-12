#include "xnor.h"

Xnor::Xnor( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  pixmapSkinName.append( ":/basic/xnor.png" );
  setOutputsOnTop( true );
  setCanChangeSkin( true );
  setPixmap( pixmapSkinName[ 0 ] );
  updatePorts( );
  setPortName( "XNOR" );
}

void Xnor::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/xnor.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
