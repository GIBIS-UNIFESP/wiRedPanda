#ifndef XOR_H
#define XOR_H

#include "graphicelement.h"

class Xor : public GraphicElement
{
public:
    explicit Xor(QGraphicsItem *parent = nullptr);
    ~Xor() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::XOR);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* XOR_H */
