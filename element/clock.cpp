#include "clock.h"

#include <QDebug>

Clock::Clock(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setRotatable(false);
  setPixmap(QPixmap(":/input/resources/input/clock 0.svg"));
  connect(&timer,&QTimer::timeout,this,&Clock::updateClock);
  setFrequency(1.0);
  setHasFrequency(true);
  on = false;
}

Clock::~Clock() {
}

void Clock::updateClock() {
  if(on) {
    on = false;
    setPixmap(QPixmap(":/input/resources/input/clock 0.svg"));
  } else {
    on = true;
    setPixmap(QPixmap(":/input/resources/input/clock 1.svg"));
  }
  setChanged(true);
  outputs().first()->setValue(on);
}

void Clock::save(QDataStream & ds) {
  GraphicElement::save(ds);
}

void Clock::load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap) {
  GraphicElement::load(ds,portMap);
}

float Clock::frequency() {
  return m_frequency;
}

void Clock::setFrequency(float freq) {
  qDebug() << "Clock frequency set to " << freq;
  if( freq > 0 && m_frequency != freq) {
    m_frequency = freq;
    timer.start( static_cast< int >(1000.0/freq) );
  }
}
