#include "inputswitch.h"

#include <QGraphicsSceneMouseEvent>

InputSwitch::InputSwitch(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setRotatable(false);
  setPixmap(QPixmap(":/input/resources/input/switch off.svg"));
  on = false;
}

InputSwitch::~InputSwitch() {

}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    if(on) {
      on = false;
      setPixmap(QPixmap(":/input/resources/input/switch off.svg"));
    } else {
      on = true;
      setPixmap(QPixmap(":/input/resources/input/switch on.svg"));
    }
    setChanged(true);
  }
}

void InputSwitch::updateLogic() {
  outputs().first()->setValue(on);
}


void InputSwitch::save(QDataStream & ds) {
  GraphicElement::save(ds);
  ds << on;
}

void InputSwitch::load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap) {
  GraphicElement::load(ds,portMap);
  ds >> on;
  if(on) {
    setPixmap(QPixmap(":/input/resources/input/switch on.svg"));
  }
}
