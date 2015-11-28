#include "display.h"
#include <QPainter>
#include <QPixmap>

Display::Display(QGraphicsItem * parent) : GraphicElement(8,8,0,0,parent) {
  setRotatable(false);
  setOutputsOnTop(true);
  updatePorts();
  setBottomPosition(58);
  setTopPosition(6);

  setPixmap(QPixmap(":/output/counter/counter_off.png"));
  a = QPixmap(":/output/counter/counter_c.png");
  b = QPixmap(":/output/counter/counter_d.png");
  c = QPixmap(":/output/counter/counter_f.png");
  d = QPixmap(":/output/counter/counter_g.png");
  e = QPixmap(":/output/counter/counter_a.png");
  f = QPixmap(":/output/counter/counter_b.png");
  g = QPixmap(":/output/counter/counter_e.png");
  h = QPixmap(":/output/counter/counter_h.png");

  setObjectName("Display");
}

Display::~Display() {

}

void Display::updateLogic() {
  update();
}

void Display::updatePorts() {
  inputs().at(0)->setPos(topPosition(), 10); //G
  inputs().at(1)->setPos(topPosition(), 25); //F
  inputs().at(2)->setPos(topPosition(), 39); //E
  inputs().at(3)->setPos(topPosition(), 54); //D
  inputs().at(4)->setPos(bottomPosition(), 10); //A
  inputs().at(5)->setPos(bottomPosition(), 25); //B
  inputs().at(6)->setPos(bottomPosition(), 39); //DP
  inputs().at(7)->setPos(bottomPosition(), 54); //C
  inputs( ).at( 0 )->setName( "G (mid)" );
  inputs( ).at( 1 )->setName( "F (upper left)" );
  inputs( ).at( 2 )->setName( "E (lower left)" );
  inputs( ).at( 3 )->setName( "D (bottom)" );
  inputs( ).at( 4 )->setName( "A (top)" );
  inputs( ).at( 5 )->setName( "B (upper right)" );
  inputs( ).at( 6 )->setName( "DP (dot)" );
  inputs( ).at( 7 )->setName( "C (lower right)" );
}

void Display::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  GraphicElement::paint(painter,option,widget);
  if(inputs().at(0)->value() == true) { //G
    painter->drawPixmap(QPoint(0,0), g);
  }
  if(inputs().at(1)->value() == true) { //F
    painter->drawPixmap(QPoint(0,0), f);
  }
  if(inputs().at(2)->value() == true) { //E
    painter->drawPixmap(QPoint(0,0), e);
  }
  if(inputs().at(3)->value() == true) { //D
    painter->drawPixmap(QPoint(0,0), d);
  }
  if(inputs().at(4)->value() == true) { //A
    painter->drawPixmap(QPoint(0,0), a);
  }
  if(inputs().at(5)->value() == true) { //B
    painter->drawPixmap(QPoint(0,0), b);
  }
  if(inputs().at(6)->value() == true) { //DP (H)
    painter->drawPixmap(QPoint(0,0), h);
  }
  if(inputs().at(7)->value() == true) { //C
    painter->drawPixmap(QPoint(0,0), c);
  }

}
