#ifndef DISPLAY_H
#define DISPLAY_H


#include <graphicelement.h>



class Display : public GraphicElement {
public:
  explicit Display(QGraphicsItem * parent);
  virtual ~Display();

public:
  virtual ElementType elementType() {
    return ElementType::DISPLAY;
  }
  virtual void updateLogic();
  void updatePorts();
  QPixmap bkg, a, b, c, d, e, f, g, h;

  // QGraphicsItem interface
public:
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
};
#endif // DISPLAY_H

