#ifndef LEDGRID_H
#define LEDGRID_H


#include <graphicelement.h>


class LedGrid : public GraphicElement {
public:
  explicit LedGrid( QGraphicsItem *parent );
  virtual ~LedGrid( );

public:
  virtual ElementType elementType( ) {
    return( ElementType::LEDGRID );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::OUTPUT );
  }
  void setColor( QString getColor );
  QString getColor( );
  virtual void refresh( );
  void updatePorts( );
  QPixmap a, b, c, d;
private:
  QString m_color;
  /* QGraphicsItem interface */
public:
  void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
  QString genericProperties( );

  /* GraphicElement interface */
public:
  void save( QDataStream &ds );
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );
};
#endif /* LedGrid_H */
