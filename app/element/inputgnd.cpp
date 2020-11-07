#include "inputgnd.h"

InputGnd::InputGnd( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  pixmapSkinName.append( ":/input/0.png" );
  setOutputsOnTop( false );
  setPixmap( pixmapSkinName[ 0 ] );
  setRotatable( false );
  setPortName( "GND" );
  outputs( ).first( )->setValue( false );
}

void InputGnd::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/input/0.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
