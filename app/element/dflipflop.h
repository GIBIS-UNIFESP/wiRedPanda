#ifndef DFLIPFLOP_H
#define DFLIPFLOP_H

#include "graphicelement.h"

class DFlipFlop : public GraphicElement
{
    bool lastClk;
    bool lastValue;

public:
    explicit DFlipFlop(QGraphicsItem *parent = nullptr);
    virtual ~DFlipFlop() override = default;

    // GraphicElement interface
public:
    virtual ElementType elementType() override
    {
        return (ElementType::DFLIPFLOP);
    }
    virtual ElementGroup elementGroup() override
    {
        return (ElementGroup::MEMORY);
    }
    virtual void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif // DFLIPFLOP_H
