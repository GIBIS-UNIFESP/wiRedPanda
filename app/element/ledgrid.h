#ifndef LEDGRID_H
#define LEDGRID_H


#include <array>
#include <graphicelement.h>


class LedGrid : public GraphicElement {
public:
  explicit LedGrid( QGraphicsItem *parent );
  virtual ~LedGrid( );
  static int current_id_number; // Number used to create distinct labels for each instance of this element.

  virtual ElementType elementType( ) override {
    return( ElementType::LEDGRID );
  }

  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::OUTPUT );
  }

  void setColor( QString getColor ) override;
  QString getColor( ) const override;
  virtual void refresh( ) override;
  void updatePorts( ) override;

  void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) override;
  QString genericProperties( ) override;

  void save( QDataStream &ds ) const override;
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) override;
  void setSkin( bool defaultSkin, QString filename ) override;

private:
  QString m_color;
  int m_colorNumber; /* white = 0, red = 2, green = 4, blue = 6, purple = 8 */
  QPixmap led_off_pixmap, led_on_pixmap;
  std::array< int, 4 > m_rowOffsets;
  std::array< int, 4 > m_colOffsets;
};
#endif /* LedGrid_H */
