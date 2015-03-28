#include "jklatch.h"

JKLatch::JKLatch(QGraphicsItem * parent) : GraphicElement(2,2,2,2, parent) {
  setPixmap(QPixmap(":/memory/resources/memory/JK-latch.svg"));
  setRotatable(false);
  updatePorts();
}

JKLatch::~JKLatch() {

}

void JKLatch::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13);
  inputs().at(1)->setPos(topPosition(),45);

  outputs().at(0)->setPos(bottomPosition(),15);
  outputs().at(1)->setPos(bottomPosition(),45);
}

