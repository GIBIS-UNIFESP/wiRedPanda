#include "inputswitch.h"

#include <QGraphicsSceneMouseEvent>

InputSwitch::InputSwitch( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  setOutputsOnTop( false );
  setRotatable( false );
  setPixmap( ":/input/switchOff.png" );
  on = false;
  setHasLabel( true );
  setHasTrigger( true );
  setPortName( "Switch" );
}

InputSwitch::~InputSwitch( ) {

}

bool InputSwitch::getOn( ) const {
  return( on );
}

void InputSwitch::setOn( bool value ) {
  on = value;
  updateLogic( );
  if( on ) {
    setPixmap( ":/input/switchOn.png" );
  }
  else {
    setPixmap( ":/input/switchOff.png" );
  }
}

void InputSwitch::mousePressEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {
    setOn( !on );

    event->accept( );
  }
  QGraphicsItem::mousePressEvent( event );
}

void InputSwitch::updateLogic( ) {
  if( !disabled( ) ) {
    output( )->setValue( on );
  }
}

void InputSwitch::save( QDataStream &ds ) const {
  GraphicElement::save( ds );
  ds << on;
}

void InputSwitch::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
  ds >> on;
  setOn( on );
  output( )->setValue( on );
}
