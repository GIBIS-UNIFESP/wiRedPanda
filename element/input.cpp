#include "input.h"

#include <QGraphicsSceneDragDropEvent>

Input::Input(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setTopPosition(50);
  setPixmap(QPixmap(":/input/resources/input/button off.svg"));
}

Input::~Input() {

}

void Input::mousePressEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton){
    setPixmap(QPixmap(":/input/resources/input/button on.svg"));
  }
}

void Input::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton){
    setPixmap(QPixmap(":/input/resources/input/button off.svg"));
  }
}
