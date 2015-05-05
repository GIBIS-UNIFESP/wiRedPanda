#ifndef JKLATCH_H
#define JKLATCH_H

#include <graphicelement.h>

class JKLatch : public GraphicElement {
public:
  explicit JKLatch(QGraphicsItem * parent);
  ~JKLatch();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::JKLATCH;
  }
  virtual void updatePorts();
};

#endif // JKLATCH_H
