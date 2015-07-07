#include "inputbutton.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
InputButton::InputButton(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setPixmap(QPixmap(":/input/resources/input/button off.svg"));
  setRotatable(false);
  outputs().first()->setValue(0);
  setBottomPosition(72);
  on = false;
  setHasLabel(true);
}

InputButton::~InputButton() {

}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/resources/input/button on.svg"));
    on = true;
    setChanged(true);
    setSelected(false);
  }
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/resources/input/button off.svg"));
    on = false;
    setChanged(true);
    setSelected(false);
  }
}

void InputButton::updateLogic() {
  if(!disabled()){
    outputs().first()->setValue(on);
  }
}
