#ifndef NOR_H
#define NOR_H

#include "graphicelement.h"

class Nor : public GraphicElement
{
public:
    explicit Nor(QGraphicsItem *parent = nullptr);
    virtual ~Nor() override = default;

    /* GraphicElement interface */
public:
    virtual ElementType elementType() override
    {
        return (ElementType::NOR);
    }
    virtual ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* NOR_H */
