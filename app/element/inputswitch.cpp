#include "inputswitch.h"

#include <QGraphicsSceneMouseEvent>

InputSwitch::InputSwitch(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setRotatable(false);
  setPixmap(QPixmap(":/input/switch off.png"));
  on = false;
  setHasLabel(true);
}

InputSwitch::~InputSwitch() {

}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent * event) {
  if(event->button() == Qt::LeftButton) {
    if(on) {
      on = false;
      setPixmap(QPixmap(":/input/switch off.png"));
    } else {
      on = true;
      setPixmap(QPixmap(":/input/switch on.png"));
    }
    setChanged(true);
    event->accept();
  }
  QGraphicsItem::mousePressEvent(event);
}

void InputSwitch::updateLogic() {
  if(!disabled()){
    outputs().first()->setValue(on);
  }
}


void InputSwitch::save(QDataStream & ds) {
  GraphicElement::save(ds);
  ds << on;
}

void InputSwitch::load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap, double version) {
  GraphicElement::load(ds,portMap,version);
  ds >> on;
  if(on) {
    setPixmap(QPixmap(":/input/switch on.png"));
  }
}
