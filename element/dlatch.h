#ifndef DLATCH_H
#define DLATCH_H

#include <graphicelement.h>

class DLatch : public GraphicElement {
public:
  DLatch(QGraphicsItem * parent);
  ~DLatch();

  // GraphicElement interface
public:
  virtual void updatePorts();
};

#endif // DLATCH_H
