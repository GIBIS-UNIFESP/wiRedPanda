#include "led.h"

#include <QDebug>
#include <QGraphicsSceneDragDropEvent>

Led::Led( QGraphicsItem *parent ) : GraphicElement( 1, 1, 0, 0, parent ) {
  setOutputsOnTop( true );
  setRotatable( false );
  setHasColors( true );
  setColor( "White" );
  setPixmap( ":/output/" + getColor( ) + "LedOff.png" );
  updatePorts( );
  setHasLabel( true );
  setPortName( "Led" );

  input( 0 )->setRequired( false );
  input( 0 )->setDefaultValue( 0 );
}

Led::~Led( ) {

}

void Led::updateLogic( ) {
  if( isValid( ) ) {
    bool value = inputs( ).first( )->value( );
    if( value == 1 ) {
      setPixmap( ":/output/" + getColor( ) + "LedOn.png" );
    }
    else {
      setPixmap( ":/output/" + getColor( ) + "LedOff.png" );
    }
  }
  else {
    setPixmap( ":/output/" + getColor( ) + "LedOff.png" );
  }
}


void Led::setColor( QString color ) {
  m_color = color;
  updateLogic( );
}

QString Led::getColor( ) {
  return( m_color );
}


void Led::save( QDataStream &ds ) {
  GraphicElement::save( ds );
  ds << getColor( );
}

void Led::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
  if( version >= 1.1 ) {
    QString clr;
    ds >> clr;
    setColor( clr );
  }
}

QString Led::genericProperties( ) {
  return( getColor( ) );
}
