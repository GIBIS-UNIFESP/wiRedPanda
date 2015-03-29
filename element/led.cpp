#include "led.h"

#include <QDebug>
#include <QGraphicsSceneDragDropEvent>

Led::Led(QGraphicsItem * parent) : GraphicElement(1,1,0,0,parent){
  setOutputsOnTop(true);
  setRotatable(false);
  setPixmap(QPixmap(":/output/resources/output/WhiteLedOff.svg"));
  updatePorts();
  setObjectName("led");
}

Led::~Led() {

}

void Led::updateLogic() {
  bool value = inputs().first()->value();
  qDebug() << "Updating led with value " << value;
  if(value == 1 ) {
    setPixmap(QPixmap(":/output/resources/output/WhiteLedOn.svg"));
  }else{
    setPixmap(QPixmap(":/output/resources/output/WhiteLedOff.svg"));
  }
}
