#include "inputvcc.h"

InputVcc::InputVcc( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  pixmapSkinName.append( ":/input/1.png" );
  setOutputsOnTop( false );
  setPixmap( pixmapSkinName[ 0 ] );
  setRotatable( false );
  setPortName( "VCC" );
  outputs( ).first( )->setValue( true );
}

void InputVcc::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/input/1.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
