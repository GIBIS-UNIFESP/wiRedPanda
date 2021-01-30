#ifndef INPUTGND_H
#define INPUTGND_H

#include "graphicelement.h"

class InputGnd : public GraphicElement
{
public:
    explicit InputGnd(QGraphicsItem *parent = nullptr);
    ~InputGnd() override = default;
    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::GND);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::STATICINPUT);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* INPUTGND_H */
