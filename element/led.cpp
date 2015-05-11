#include "led.h"

#include <QDebug>
#include <QGraphicsSceneDragDropEvent>

Led::Led(QGraphicsItem * parent) : GraphicElement(1,1,0,0,parent){
  setOutputsOnTop(true);
  setRotatable(false);
  setPixmap(QPixmap(":/output/resources/output/WhiteLedOff.svg"));
  updatePorts();
}

Led::~Led() {

}



void Led::updateLogic() {

  if(isValid()) {
    bool value = inputs().first()->value();
    if(value == 1 ) {
      setPixmap(QPixmap(":/output/resources/output/WhiteLedOn.svg"));
    }else{
      setPixmap(QPixmap(":/output/resources/output/WhiteLedOff.svg"));
    }
  }
}
