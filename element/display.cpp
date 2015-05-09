#include "display.h"
#include <QPixmap>

Display::Display(QGraphicsItem * parent) : GraphicElement(8,8,0,0,parent) {

        setPixmap(QPixmap(":/output/counter/resources/output/counter/counter off.svg"));
        setRotatable(false);
        setOutputsOnTop(true);
        updatePorts();
}

Display::~Display(){

}

void Display::updateLogic() {
        if(inputs().at(0)->value() == true){//A
            setPixmap(QPixmap(":/output/counter/resources/output/counter/counter c.svg"));
        }
        if(inputs().at(1)->value() == true){//B
            setPixmap(QPixmap(":/output/counter/resources/output/counter/counter d.svg"));
        }
        if(inputs().at(2)->value() == true){//C
            setPixmap(QPixmap(":/output/counter/resources/output/counter/counter f.svg"));
        }
        if(inputs().at(3)->value() == true){//D
            setPixmap(QPixmap(":/output/counter/resources/output/counter/counter g.svg"));
        }
        if(inputs().at(4)->value() == true){//E
            setPixmap(QPixmap(":/output/counter/resources/output/counter/counter a.svg"));
        }
        if(inputs().at(5)->value() == true){//F
            setPixmap(QPixmap(":/output/counter/resources/output/counter/counter b.svg"));
        }
        if(inputs().at(6)->value() == true){//G
            setPixmap(QPixmap(":/output/counter/resources/output/counter/counter e.svg"));
        }
}


/*
void Led::updateLogic() {

  if(isValid()) {
    bool value = inputs().first()->value();
    if(value == 1 ) {
      setPixmap(QPixmap(":/output/resources/output/WhiteLedOn.svg"));
    }else{
      setPixmap(QPixmap(":/output/resources/output/WhiteLedOff.svg"));
    }
  }
}*/


