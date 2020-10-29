#include "clock.h"

#include <QDebug>

bool Clock::reset = false;

Clock::Clock( QGraphicsItem *parent ) : GraphicElement( 0, 0, 1, 1, parent ) {
  setOutputsOnTop( false );
  setRotatable( false );
  /*  connect(&timer,&QTimer::timeout,this,&Clock::updateClock); */
  setFrequency( 1.0 ); // TODO: call to virtual function during construction
  setHasFrequency( true );
  on = false;
  Clock::reset = true;
  setHasLabel( true );
  setPortName( "Clock" );
  setOn( 0 );
}

void Clock::updateClock( ) {
  if( !disabled( ) ) {
    elapsed++;
    if( ( elapsed % interval ) == 0 ) {
      setOn( !on );
    }
  }
  setOn( on );
}

bool Clock::getOn( ) const {
  return( on );
}

void Clock::setOn( bool value ) {
  on = value;
  if( on ) {
    setPixmap( ":/input/clock1.png" );
  }
  else {
    setPixmap( ":/input/clock0.png" );
  }
  m_outputs.first( )->setValue( on );
}

void Clock::save( QDataStream &ds ) const {
  GraphicElement::save( ds );
  ds << getFrequency( );
}

void Clock::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
  if( version >= 1.1 ) {
    float freq;
    ds >> freq;
    setFrequency( freq );
  }
}

float Clock::getFrequency( ) const {
  return( static_cast< float >( m_frequency ) );
}

void Clock::setFrequency( float freq ) {
  /*  qDebug() << "Clock frequency set to " << freq; */
  if( !qFuzzyIsNull( freq ) ) {
    int auxinterval = 1000 / ( freq * GLOBALCLK );
    if( auxinterval > 0 ) {
      interval = auxinterval;
      m_frequency = static_cast< double >( freq );
      elapsed = 0;
      Clock::reset = true;
      //      qDebug() << "Freq = " << freq <<  " interval = " << interval;
    }
    /*    timer.start( static_cast< int >(1000.0/freq) ); */
  }
}

void Clock::resetClock( ) {
  setOn( true );
  elapsed = 0;
}

QString Clock::genericProperties( ) {
  return( QString( "%1 Hz" ).arg( static_cast< double >( getFrequency( ) ) ) );
}