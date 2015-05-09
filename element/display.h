#ifndef DISPLAY_H
#define DISPLAY_H


#include <graphicelement.h>



class Display : public GraphicElement {
public:
  explicit Display(QGraphicsItem * parent);
  ~Display();

public:
  virtual ElementType elementType() {
    return ElementType::DISPLAY;
  }
  void updateLogic();
};
#endif // DISPLAY_H

