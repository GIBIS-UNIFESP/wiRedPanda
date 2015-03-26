#include "nand.h"

Nand::Nand(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent){
    setOutputsOnTop(true);
    setPixmap(QPixmap(":/basic/resources/basic/nand.svg"));
    updatePorts();
}

Nand::~Nand()
{

}

