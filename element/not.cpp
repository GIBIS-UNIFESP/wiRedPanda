#include "not.h"

Not::Not(QGraphicsItem *parent) : GraphicElement(1,1,1,1){
   setOutputsOnTop(true);
   setPixmap(QPixmap(":/basic/resources/basic/not.svg"));
   updatePorts();
}

Not::~Not()
{

}


