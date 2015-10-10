#include "inputbutton.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
InputButton::InputButton(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setPixmap(QPixmap(":/input/buttonOff.png"));
  setRotatable(false);
  outputs().first()->setValue(0);
  on = false;
  setHasLabel(true);
  setObjectName("Button");
}

InputButton::~InputButton() {

}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/buttonOn.png"));
    on = true;
    setChanged(true);
    event->accept();
  }
  QGraphicsItem::mousePressEvent(event);
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/buttonOff.png"));
    on = false;
    setChanged(true);
    event->accept();
  }
  QGraphicsItem::mouseReleaseEvent(event);
}

void InputButton::updateLogic() {
  if(!disabled()){
    outputs().first()->setValue(on);
  }
}
