#ifndef XNOR_H
#define XNOR_H

#include "graphicelement.h"

class Xnor : public GraphicElement
{
public:
    explicit Xnor(QGraphicsItem *parent = nullptr);
    ~Xnor() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::XNOR);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* Xnor_H */
