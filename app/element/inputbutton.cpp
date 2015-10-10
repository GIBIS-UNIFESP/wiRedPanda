#include "inputbutton.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
InputButton::InputButton(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setPixmap(QPixmap(":/input/button off.png"));
  setRotatable(false);
  outputs().first()->setValue(0);
  on = false;
  setHasLabel(true);
}

InputButton::~InputButton() {

}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/button on.png"));
    on = true;
    setChanged(true);
    event->accept();
  }
  QGraphicsItem::mousePressEvent(event);
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    setPixmap(QPixmap(":/input/button off.png"));
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
