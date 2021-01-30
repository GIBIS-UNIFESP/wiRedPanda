#ifndef JKLATCH_H
#define JKLATCH_H

#include "graphicelement.h"

class JKLatch : public GraphicElement
{
public:
    explicit JKLatch(QGraphicsItem *parent = nullptr);
    ~JKLatch() override = default;

    /* GraphicElement interface */
public:
    ElementType elementType() override
    {
        return (ElementType::JKLATCH);
    }
    ElementGroup elementGroup() override
    {
        return (ElementGroup::MEMORY);
    }
    void updatePorts() override;
};

#endif /* JKLATCH_H */
