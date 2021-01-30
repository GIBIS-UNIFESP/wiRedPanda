#ifndef LOGICNODE_H
#define LOGICNODE_H

#include "logicelement.h"

class LogicNode : public LogicElement
{
public:
    explicit LogicNode();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &) override;
};

#endif // LOGICNODE_H