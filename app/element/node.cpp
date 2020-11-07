#include "node.h"

#include <QPainter>

Node::Node( QGraphicsItem *parent ) : GraphicElement( 1, 1, 1, 1, parent ) {
  pixmapSkinName.append( ":/basic/node.png" );
  setPixmap( pixmapSkinName[ 0 ], QRect( QPoint( 16, 16 ), QPoint( 48, 48 ) ) );
  updatePorts( );
  setPortName( "NODE" );
  input( )->setRequired( true );
}

void Node::updatePorts( ) {
  input( )->setPos( 0, 16 );
  output( )->setPos( 32, 16 );
}

ElementType Node::elementType( ) {
  return( ElementType::NODE );
}

void Node::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin )
    pixmapSkinName[ 0 ] = ":/basic/node.png";
  else
    pixmapSkinName[ 0 ] = filename;
  setPixmap( pixmapSkinName[ 0 ] );
}
