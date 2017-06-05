#ifndef BUZZER_H
#define BUZZER_H

#include "graphicelement.h"
#include <QSound>

class Buzzer : public GraphicElement {
public:
  explicit Buzzer( QGraphicsItem *parent = 0 );
  virtual ~Buzzer( );
public:
  /* GraphicElement interface */
  virtual ElementType elementType( ) {
    return( ElementType::BUZZER );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::OUTPUT );
  }
  void updateLogic( );

  void setAudio( QString note );
  QString getAudio( );

  void mute( bool _mute = true );

private:
  void playbuzzer( );
  void stopbuzzer( );
  int play;
  QSound *m_audio;
  QString m_note;
  bool m_muted;


  // GraphicElement interface
public:
  void save( QDataStream &ds );
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );
};

#endif // BUZZER_H
