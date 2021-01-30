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
    ~TFlipFlop() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::TFLIPFLOP);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::MEMORY);
    }
    void updatePorts() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* TFLIPFLOP_H */
