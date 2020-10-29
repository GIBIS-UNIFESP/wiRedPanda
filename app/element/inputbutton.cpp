#include "inputbutton.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>

InputButton::InputButton( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  setOutputsOnTop( false );
  setPixmap( ":/input/buttonOff.png" );
  setRotatable( false );
  m_outputs.first( )->setValue( 0 );
  setOn( false );
  setHasLabel( true );
  setHasTrigger( true );
  setPortName( "Button" );
}

void InputButton::mousePressEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {
    setOn( true );

    event->accept( );
  }
  QGraphicsItem::mousePressEvent( event );
}

void InputButton::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {

    setOn( false );

    event->accept( );
  }
  GraphicElement::mouseReleaseEvent( event );
}

void InputButton::updateLogic( ) {
  if( !disabled( ) ) {
    output( )->setValue( on );
  }
}

bool InputButton::getOn( ) const {
  return( on );
}

void InputButton::setOn( const bool value ) {
  on = value;
  setPixmap( on ? ":/input/buttonOn.png" : ":/input/buttonOff.png" );
  updateLogic( );
}