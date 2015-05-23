#include "led.h"

#include <QDebug>
#include <QGraphicsSceneDragDropEvent>

Led::Led(QGraphicsItem * parent) : GraphicElement(1,1,0,0,parent) {
  setOutputsOnTop(true);
  setRotatable(false);
  setHasColors(true);
  setColor("White");
  setPixmap(QPixmap(":/output/resources/output/" + color() + "LedOff.svg"));
  updatePorts();
}

Led::~Led() {

}

void Led::updateLogic() {
  if(isValid()) {
    bool value = inputs().first()->value();
    if(value == 1 ) {
      setPixmap(QPixmap(":/output/resources/output/" + color() + "LedOn.svg"));
    } else {
      setPixmap(QPixmap(":/output/resources/output/"  + color() + "LedOff.svg"));
    }
  } else {
    setPixmap(QPixmap(":/output/resources/output/"  + color() + "LedOff.svg"));
  }
}


void Led::setColor(QString color) {
  m_color = color;
  updateLogic();
}

QString Led::color() {
  return m_color;
}


void Led::save(QDataStream & ds) {
  GraphicElement::save(ds);
  ds << color();
}

void Led::load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap) {
  GraphicElement::load(ds,portMap);
  QString clr;
  ds >> clr;
  setColor(clr);
}
