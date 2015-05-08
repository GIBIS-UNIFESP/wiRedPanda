#include "jkflipflop.h"

JKFlipFlop::JKFlipFlop(QGraphicsItem * parent) : GraphicElement(5,5,2,2,parent) {
    setPixmap(QPixmap(":/memory/resources/memory/JK-flipflop.svg"));
    setRotatable(false);
    updatePorts();
}

JKFlipFlop::~JKFlipFlop() {

}

void JKFlipFlop::updatePorts() {
    inputs().at(0)->setPos(topPosition(),13);
    inputs().at(1)->setPos(topPosition(),29);
    inputs().at(2)->setPos(topPosition(),45);
    inputs().at(3)->setPos(32,topPosition());
    inputs().at(4)->setPos(32,bottomPosition());

    outputs().at(0)->setPos(bottomPosition(),15);
    outputs().at(1)->setPos(bottomPosition(),45);
}

void JKFlipFlop::updateLogic() {
    char res = outputs().first()->value(); //Output 1
    if(!isValid()) {
        res = -1;
    } else {
        if(inputs().at(1)->value() == true){    //If Clock
            //JK Logic
            if(inputs().at(2)->value() == inputs().at(3)->value()){ //IF J=K
                res = !res;
            } else if (inputs().at(2)->value() == true){ //J
                res = true;
            } else if (inputs().at(3)->value() == true){ //K
                res = false;
            }
        }
        if (inputs().at(2)->value() == true){ //Set
            res = true;
        }
        if (inputs().at(3)->value() == true){ //Reset
            res = false;
        }
    }
    outputs().first()->setValue(res);
    outputs().last()->setValue(!res);
}
