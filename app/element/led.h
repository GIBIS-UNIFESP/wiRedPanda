#ifndef LED_H
#define LED_H

#include "graphicelement.h"

class Led : public GraphicElement {
public:
  explicit Led( QGraphicsItem *parent = nullptr );
  virtual ~Led( ) override = default;
  static int current_id_number; // Number used to create distinct labels for each instance of this element.

  /* GraphicElement interface */
  virtual ElementType elementType( ) override {
    return( ElementType::LED );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::OUTPUT );
  }
  void refresh( ) override;
  void setColor( QString getColor ) override;
  QString getColor( ) const override;

private:
  QString m_color;


  /* GraphicElement interface */
public:
  void save( QDataStream &ds ) const override;
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) override;
  QString genericProperties( ) override;

  // GraphicElement interface
public:
  void updatePorts( ) override;
};

#endif /* LED_H */