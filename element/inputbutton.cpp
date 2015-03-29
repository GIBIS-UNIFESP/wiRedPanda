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
  setObjectName("inputButton");
}

InputButton::~InputButton() {

}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/resources/input/button on.svg"));
    on = true;
    setChanged(true);
  }
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/resources/input/button off.svg"));
    on = false;
    setChanged(true);
  }
}

void InputButton::updateLogic() {
  qDebug() << "Updating input with value = " << on;
  outputs().first()->setValue(on);
}
