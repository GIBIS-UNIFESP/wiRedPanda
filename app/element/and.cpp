#include "and.h"

And::And( QGraphicsItem *parent ) : GraphicElement( 2, 8, 1, 1, parent ) {
  pixmapSkinName.append( ":/basic/and.png" );
  setOutputsOnTop( true );
  setPixmap( pixmapSkinName[ 0 ] );
  updatePorts( );
  setCanChangeSkin( true );
  setPortName( "AND" );
  setToolTip( "AND" );
}

void And::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/and.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
