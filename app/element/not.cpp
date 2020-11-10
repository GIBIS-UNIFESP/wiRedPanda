#include "not.h"

Not::Not( QGraphicsItem *parent ) : GraphicElement( 1, 1, 1, 1, parent ) {
  pixmapSkinName.append( ":/basic/not.png" );
  setOutputsOnTop( true );
  setPixmap( pixmapSkinName[ 0 ] );
  setCanChangeSkin( true );
  updatePorts( );
  setPortName( "NOT" );
}

void Not::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/not.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
