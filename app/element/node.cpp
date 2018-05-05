#include "node.h"

#include <QPainter>

Node::Node( QGraphicsItem *parent ) : GraphicElement( 1, 1, 1, 1, parent ) {
//  QPixmap pix( 32, 32 );
//  pix.fill( QColor( 0, 0, 0, 0 ) );
//  QPainter *paint = new QPainter( &pix );
//  paint->setBrush( Qt::gray );
///*  paint->drawRect(8,8,16,16); */
//  paint->drawEllipse( 4, 4, 24, 24 );
//  delete paint;
//  setPixmap( pix );
  setPixmap( ":/basic/node.png", QRect( QPoint( 16, 16 ), QPoint( 48, 48 ) ) );
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
