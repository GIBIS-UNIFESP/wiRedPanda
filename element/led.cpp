#include "led.h"

#include <QGraphicsSceneDragDropEvent>

Led::Led(QGraphicsItem * parent) : GraphicElement(1,1,0,0,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/output/resources/output/blue led off.svg"));
  updatePorts();
}

Led::~Led() {

}

//void Led::mousePressEvent(QGraphicsSceneMouseEvent * event) {
//  if(event->button() == Qt::LeftButton) {
//    setPixmap(QPixmap(":/output/resources/output/blue led on.svg"));
//  }
//}

//void Led::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
//  if(event->button() == Qt::LeftButton) {
//    setPixmap(QPixmap(":/output/resources/output/blue led off.svg"));
//  }
//}
