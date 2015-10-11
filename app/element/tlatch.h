#ifndef TLATCH_H
#define TLATCH_H

#include <graphicelement.h>

class TLatch : public GraphicElement {
public:
  explicit TLatch(QGraphicsItem * parent);
  virtual ~TLatch();

  // GraphicElement interface
public:
    virtual ElementType elementType() {
      return ElementType::TLATCH;
    }
  virtual void updatePorts();
  void updateLogic();
};

#endif // TLATCH_H
