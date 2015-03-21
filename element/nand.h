#ifndef NAND_H
#define NAND_H

#include "graphicelement.h"


class Nand : public GraphicElement{
public:
    explicit Nand(QGraphicsItem *parent = 0);
    ~Nand();
};

#endif // NAND_H

