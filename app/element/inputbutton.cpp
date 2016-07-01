#include "inputbutton.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
InputButton::InputButton( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  setOutputsOnTop( false );
  setPixmap( ":/input/buttonOff.png" );
  setRotatable( false );
  outputs( ).first( )->setValue( 0 );
  setOn( false );
  setHasLabel( true );
  setHasTrigger( true );
  setPortName( "Button" );
}

InputButton::~InputButton( ) {

}

void InputButton::mousePressEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {
    setOn( true );
    setChanged( true );
    event->accept( );
  }
  QGraphicsItem::mousePressEvent( event );
}

void InputButton::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {

    setOn( false );
    setChanged( true );
    event->accept( );
  }
  GraphicElement::mouseReleaseEvent( event );
}

void InputButton::updateLogic( ) {
  if( !disabled( ) ) {
    outputs( ).first( )->setValue( on );
  }
}

bool InputButton::getOn( ) const {
  return( on );
}

void InputButton::setOn( bool value ) {
  on = value;
  if( on ) {
    setPixmap( ":/input/buttonOn.png" );
  }
  else {
    setPixmap( ":/input/buttonOff.png" );
  }
  updateLogic( );
}
