#ifndef LED_H
#define LED_H

#include "graphicelement.h"

class Led : public GraphicElement {
public:
  explicit Led( QGraphicsItem *parent = 0 );
  virtual ~Led( );

  /* GraphicElement interface */
  virtual ElementType elementType( ) {
    return( ElementType::LED );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::OUTPUT );
  }
  void updateLogic( );
  void setColor( QString getColor );
  QString getColor( );

private:
  QString m_color;


  /* GraphicElement interface */
public:
  void save( QDataStream &ds );
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );
  QString genericProperties( );
};

#endif /* LED_H */
