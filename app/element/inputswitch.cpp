#include "inputswitch.h"

#include <QGraphicsSceneMouseEvent>

int InputSwitch::current_id_number = 0;

InputSwitch::InputSwitch( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  pixmapSkinName.append( ":/input/switchOff.png" );
  pixmapSkinName.append( ":/input/switchOn.png" );
  setOutputsOnTop( false );
  setCanChangeSkin( true );
  setRotatable( false );
  setPixmap( pixmapSkinName[ 0 ] );
  on = false;
  setHasLabel( true );
  setHasTrigger( true );
  setPortName( "Switch" );
}

bool InputSwitch::getOn( ) const {
  return( on );
}

void InputSwitch::setOn( bool value ) {
  on = value;
  if( !disabled( ) ) {
    output( )->setValue( on );
  }
  if( on ) {
    setPixmap( pixmapSkinName[ 1 ] );
  }
  else {
    setPixmap( pixmapSkinName[ 0 ] );
  }
}

void InputSwitch::mousePressEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {
    setOn( !on );

    event->accept( );
  }
  QGraphicsItem::mousePressEvent( event );
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

void InputSwitch::setSkin( bool defaultSkin, QString filename ) {
  if( defaultSkin ) {
    if( !on ) {
      pixmapSkinName[ 0 ] = ":/input/switchOff.png";
      setPixmap( pixmapSkinName[ 0 ] );
    }
    else {
      pixmapSkinName[ 1 ] = ":/input/switchOn.png";
      setPixmap( pixmapSkinName[ 1 ] );
    }
  }
  else {
    if( !on ) {
      pixmapSkinName[ 0 ] = filename;
      setPixmap( pixmapSkinName[ 0 ] );
    }
    else {
      pixmapSkinName[ 1 ] = filename;
      setPixmap( pixmapSkinName[ 1 ] );
    }
  }
}
