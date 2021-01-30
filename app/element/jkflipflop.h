#ifndef JKFLIPFLOP_H
#define JKFLIPFLOP_H

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement
{
    bool lastClk;

public:
    explicit JKFlipFlop(QGraphicsItem *parent = nullptr);
    ~JKFlipFlop() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::JKFLIPFLOP);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::MEMORY);
    }
    void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* JKFLIPFLOP_H */
