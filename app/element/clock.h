#ifndef CLOCK_H
#define CLOCK_H

#include "globalproperties.h"
#include "graphicelement.h"
#include "input.h"
#include <QTime>

class Clock : public GraphicElement, public Input {
  int interval;
  int elapsed;
  bool on;
  double m_frequency;

public:
  explicit Clock( QGraphicsItem *parent );
  virtual ~Clock( );
  static bool reset;
public slots:
  virtual ElementType elementType( ) {
    return( ElementType::CLOCK );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::INPUT );
  }
//  void updateClock();

  // GraphicElement interface
public:
  void save( QDataStream &ds );
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );
  float getFrequency( );
  void setFrequency( float freq );
  void updateLogic( );
  void resetClock( );
  QString genericProperties( );
private:
  void updateClock( );


  // Input interface
public:
  bool getOn( ) const;
  void setOn( bool value );
};

#endif // CLOCK_H
