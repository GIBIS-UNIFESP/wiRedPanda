#ifndef TFLIPFLOP_H
#define TFLIPFLOP_H

#include "graphicelement.h"

class TFlipFlop : public GraphicElement
{
    bool lastClk;
    signed char lastT;
    signed char lastQ;

public:
    explicit TFlipFlop(QGraphicsItem *parent = nullptr);
    virtual ~TFlipFlop() override = default;

    /* GraphicElement interface */
public:
    virtual ElementType elementType() override
    {
        return (ElementType::TFLIPFLOP);
    }
    virtual ElementGroup elementGroup() override
    {
        return (ElementGroup::MEMORY);
    }
    virtual void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* TFLIPFLOP_H */
