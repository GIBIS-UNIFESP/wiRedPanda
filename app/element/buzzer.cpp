#include "buzzer.h"

#include <QDebug>
#include <QGraphicsSceneDragDropEvent>

Buzzer::Buzzer( QGraphicsItem *parent ) : GraphicElement( 1, 1, 0, 0, parent ) {
  setOutputsOnTop( true );
  setRotatable( false );
  setHasAudio( true );
  setPixmap( ":/output/BuzzerOff.png" );
  updatePorts( );
  setHasLabel( true );
  setPortName( "Buzzer" );
  setAudio( "C6" );
  play = 0;
}

Buzzer::~Buzzer( ) {

}

void Buzzer::refresh( ) {
  if( isValid( ) ) {
    bool value = inputs( ).first( )->value( );
    if( value == 1 ) {
      playbuzzer( );

    }
    else {
      if( play == 1 ) {
        stopbuzzer( );
      }
    }
  }
  else {
    stopbuzzer( );
  }
}

void Buzzer::setAudio( QString note ) {
  m_audio.setSource( QUrl::fromLocalFile( QString( ":output/audio/%1.wav" ).arg( note ) ) );
  m_audio.setVolume( 0.35 );
  m_audio.setLoopCount( QSoundEffect::Infinite );
  m_note = note;
}

QString Buzzer::getAudio( ) {
  return( m_note );
}

void Buzzer::mute( bool _mute ) {
  m_audio.setMuted( _mute );
}

void Buzzer::playbuzzer( ) {
  if( ( play == 0 ) ) {
    setPixmap( ":/output/BuzzerOn.png" );
    m_audio.play( );
  }
  play = 1;
}

void Buzzer::stopbuzzer( ) {
  setPixmap( ":/output/BuzzerOff.png" );
  play = 0;
  m_audio.stop( );
}

void Buzzer::save( QDataStream &ds ) {
  GraphicElement::save( ds );
  ds << getAudio( );
}

void Buzzer::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
  if( version >= 2.4 ) {
    QString note;
    ds >> note;
    setAudio( note );
  }
}
