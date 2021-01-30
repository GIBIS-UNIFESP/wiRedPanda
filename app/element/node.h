#ifndef NODE_H
#define NODE_H

#include "graphicelement.h"

class Node : public GraphicElement
{
public:
    explicit Node(QGraphicsItem *parent = nullptr);
    ~Node() override = default;

    void updatePorts() override;
    ElementGroup elementGroup() override
    {
        return (ElementGroup::GATE);
    }
    /* GraphicElement interface */
public:
    ElementType elementType() override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* NODE_H */
