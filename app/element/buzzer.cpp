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
  m_audio = nullptr;
  m_muted = false;
  setAudio( "C6" );
  play = 0;
}

Buzzer::~Buzzer( ) {
  if( m_audio ) {
    m_audio->stop( );
    delete m_audio;
  }
}

void Buzzer::updateLogic( ) {
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
  if( m_audio && ( m_note != note ) ) {
    delete m_audio;
    m_audio = nullptr;
  }
  m_note = note;
  if( m_audio == nullptr ) {
    m_audio = new QSound( QString( ":output/audio/%1.wav" ).arg( note ) );
    m_audio->setLoops( -1 );
  }
}

QString Buzzer::getAudio( ) {
  return( m_note );
}

void Buzzer::mute( bool _mute ) {
  if( m_audio && _mute ) {
    m_audio->stop( );

  }
  m_muted = _mute;
}

void Buzzer::playbuzzer( ) {
  if( ( play == 0 ) ) {
    setPixmap( ":/output/BuzzerOn.png" );
    if( !m_muted ) {
      m_audio->play( );
    }
  }
  play = 1;
}

void Buzzer::stopbuzzer( ) {
  setPixmap( ":/output/BuzzerOff.png" );
  play = 0;
  m_audio->stop( );
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
