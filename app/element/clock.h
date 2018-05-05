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
  virtual ~Clock( ) override;
  static bool reset;
public slots:
  virtual ElementType elementType( ) const override {
    return( ElementType::CLOCK );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::INPUT );
  }
//  void updateClock();

  // GraphicElement interface
public:
  void save( QDataStream &ds ) const override;
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) override;
  double getFrequency( ) const override;
  void setFrequency(double freq ) override;
  void updateClock( );
  void resetClock( );
  QString genericProperties( ) override;
public:
  bool getOn( ) const override;
  void setOn( bool value ) override;
};


#endif // CLOCK_H
