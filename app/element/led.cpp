#include "led.h"
#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <bitset>

Led::Led( QGraphicsItem *parent ) : GraphicElement( 1, 4, 0, 0, parent ) {
  setOutputsOnTop( true );
  setRotatable( false );
  setHasColors( true );
  setColor( "White" );
  setPixmap( ":/output/" + getColor( ) + "LedOff.png" );
  updatePorts( );
  setHasLabel( true );
  setPortName( "Led" );

}

Led::~Led( ) {

}

QVector< QString > led_2bits = {
  ":/output/16colors/BlackLedOn.png", /* 00 */
  ":/output/16colors/RedLedOn.png", /* 01 */
  ":/output/16colors/GreenLedOn.png", /* 10 */
  ":/output/16colors/RoyalLedOn.png" /* 11 */
};

QVector< QString > led_3bits = {
  ":/output/16colors/BlackLedOn.png", /* 000 */
  ":/output/16colors/RoyalLedOn.png", /* 001 */
  ":/output/16colors/GreenLedOn.png", /* 010 */
  ":/output/16colors/AquaLightLedOn.png", /* 011 */
  ":/output/16colors/RedLedOn.png", /* 100 */
  ":/output/16colors/MagentaLedOn.png", /* 101 */
  ":/output/16colors/YellowLedOn.png", /* 110 */
  ":/output/16colors/WhiteLedOn.png", /* 111 */
};

QVector< QString > led_4bits = {
  ":/output/16colors/WhiteLedOn.png",
  ":/output/16colors/BlackLedOn.png",
  ":/output/16colors/NavyBlueLedOn.png",
  ":/output/16colors/GreenLedOn.png",
  ":/output/16colors/RedLedOn.png",
  ":/output/16colors/DarkRedLedOn.png",
  ":/output/16colors/PurpleLedOn.png",
  ":/output/16colors/OrangeLedOn.png",
  ":/output/16colors/YellowLedOn.png",
  ":/output/16colors/LimeGreenLedOn.png",
  ":/output/16colors/TealLedOn.png",
  ":/output/16colors/AquaLightLedOn.png",
  ":/output/16colors/RoyalLedOn.png",
  ":/output/16colors/HotPinkLedOn.png",
  ":/output/16colors/DarkGrayLedOn.png",
  ":/output/16colors/LightGrayLedOn.png",
};

void Led::refresh( ) {
  std::bitset< 4 > index;
  for( int i = 0; i < inputSize( ); ++i ) {
    index[ i ] = input( inputSize( ) - i - 1 )->value( );
  }
  int idx = index.to_ulong( );
  if( isValid( ) ) {
    switch( inputSize( ) ) {
        case 1: /* 1 bit */
        if( input( 0 )->value( ) == 1 ) {
          setPixmap( ":/output/" + getColor( ) + "LedOn.png" );
        }
        else {
          setPixmap( ":/output/" + getColor( ) + "LedOff.png" );
        }
        break;
        case 2: {
        /* 2 bits */
        setPixmap( led_2bits[ idx ] );
        break;
      }
        case 3: {
        /* 3 bits */
        setPixmap( led_3bits[ idx ] );
        break;
      }
        case 4: {
        /* 4 bits */
        setPixmap( led_4bits[ idx ] );
        break;
      }
    }
  }
  else {
    setPixmap( ":/output/WhiteLedOff.png" );
  }
}

void Led::setColor( QString color ) {
  m_color = color;
  refresh( );
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

void Led::updatePorts( ) {
  setHasColors( inputSize( ) == 1 );

  GraphicElement::updatePorts( );
}
