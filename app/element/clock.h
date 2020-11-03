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
  explicit Clock( QGraphicsItem *parent = nullptr );
  virtual ~Clock( );

  static bool reset;
public slots:
  virtual ElementType elementType( ) override
  {
    return( ElementType::CLOCK );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::INPUT );
  }
//  void updateClock();

  // GraphicElement interface
public:
  void save( QDataStream &ds ) const override;
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) override;
  float getFrequency( ) const override;
  void setFrequency( float freq ) override;
  void updateClock( );
  void resetClock( );
  QString genericProperties( ) override;
public:
  bool getOn( ) const override;
  void setOn( bool value ) override;
};


#endif // CLOCK_H
