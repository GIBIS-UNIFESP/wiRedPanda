#include "clock.h"

Clock::Clock(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setRotatable(false);
  setPixmap(QPixmap(":/input/resources/input/clock 0.svg"));
  connect(&timer,&QTimer::timeout,this,&Clock::updateClock);
  timer.start(500);
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
}

