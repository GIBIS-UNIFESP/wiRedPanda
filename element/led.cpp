#include "led.h"

#include <QGraphicsSceneDragDropEvent>

Led::Led(QGraphicsItem * parent) : GraphicElement(1,1,0,0,parent) {
  setOutputsOnTop(true);
  setRotatable(false);
  setPixmap(QPixmap(":/output/resources/output/WhiteLedOff.svg"));
  updatePorts();
}

Led::~Led() {

}
