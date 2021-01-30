#ifndef AND_H
#define AND_H

#include "graphicelement.h"

class And : public GraphicElement
{
public:
    explicit And(QGraphicsItem *parent = nullptr);
    virtual ~And() override = default;

    /* GraphicElement interface */
public:
    virtual ElementType elementType() override
    {
        return (ElementType::AND);
    }
    virtual ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* AND_H */
