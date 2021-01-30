#ifndef NAND_H
#define NAND_H

#include "graphicelement.h"

class Nand : public GraphicElement
{
public:
    explicit Nand(QGraphicsItem *parent = nullptr);
    ~Nand() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::NAND);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* NAND_H */
