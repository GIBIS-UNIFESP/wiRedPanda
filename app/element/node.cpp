#include "node.h"

#include <QPainter>

Node::Node( QGraphicsItem *parent ) : GraphicElement( 1, 1, 1, 1, parent ) {
  QPixmap pix( 32, 32 );
  pix.fill( QColor( 0, 0, 0, 0 ) );
  QPainter *paint = new QPainter( &pix );
  paint->setBrush( Qt::gray );
/*  paint->drawRect(8,8,16,16); */
  paint->drawEllipse( 4, 4, 24, 24 );
  delete paint;
  setPixmap( pix );
  updatePorts( );
  setObjectName( "NODE" );
}

void Node::updatePorts( ) {
  inputs( ).front( )->setPos( 0, 16 );
  outputs( ).front( )->setPos( 32, 16 );
}

void Node::updateLogic( ) {
  outputs( ).front( )->setValue( inputs( ).first()->value( ) );
}


ElementType Node::elementType( ) {
  return ElementType::NODE;
}
