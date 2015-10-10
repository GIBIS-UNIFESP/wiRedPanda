#include "display.h"
#include <QPainter>
#include <QPixmap>

Display::Display(QGraphicsItem * parent) : GraphicElement(8,8,0,0,parent) {
  setRotatable(false);
  setOutputsOnTop(true);
  updatePorts();
  setBottomPosition(58);
  setTopPosition(6);

  setPixmap(QPixmap(":/output/counter/counter off.png"));
  a = QPixmap(":/output/counter/counter c.png");
  b = QPixmap(":/output/counter/counter d.png");
  c = QPixmap(":/output/counter/counter f.png");
  d = QPixmap(":/output/counter/counter g.png");
  e = QPixmap(":/output/counter/counter a.png");
  f = QPixmap(":/output/counter/counter b.png");
  g = QPixmap(":/output/counter/counter e.png");
  h = QPixmap(":/output/counter/counter h.png");
}

Display::~Display() {

}

void Display::updateLogic() {
  update();
}

void Display::updatePorts() {
  inputs().at(0)->setPos(topPosition(), 10); //A
  inputs().at(1)->setPos(topPosition(), 25); //B
  inputs().at(2)->setPos(topPosition(), 39); //C
  inputs().at(3)->setPos(topPosition(), 54); //D
  inputs().at(4)->setPos(bottomPosition(), 10); //E
  inputs().at(5)->setPos(bottomPosition(), 25); //F
  inputs().at(6)->setPos(bottomPosition(), 39); //G
  inputs().at(7)->setPos(bottomPosition(), 54); //DOT
}

void Display::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  GraphicElement::paint(painter,option,widget);
  if(inputs().at(0)->value() == true) { //A
    painter->drawPixmap(QPoint(0,0), a);
  }
  if(inputs().at(1)->value() == true) { //B
    painter->drawPixmap(QPoint(0,0), b);
  }
  if(inputs().at(2)->value() == true) { //C
    painter->drawPixmap(QPoint(0,0), c);
  }
  if(inputs().at(3)->value() == true) { //D
    painter->drawPixmap(QPoint(0,0), d);
  }
  if(inputs().at(4)->value() == true) { //E
    painter->drawPixmap(QPoint(0,0), e);
  }
  if(inputs().at(5)->value() == true) { //F
    painter->drawPixmap(QPoint(0,0), f);
  }
  if(inputs().at(6)->value() == true) { //G
    painter->drawPixmap(QPoint(0,0), g);
  }
  if(inputs().at(7)->value() == true) { //DOT ( H )
    painter->drawPixmap(QPoint(0,0), h);
  }

}
