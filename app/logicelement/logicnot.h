#ifndef LOGICNOT_H
#define LOGICNOT_H

#include "logicelement.h"

class LogicNot : public LogicElement
{
public:
    explicit LogicNot();

    /* LogicElement interface */
protected:
    virtual void _updateLogic(const std::vector<bool> &inputs);
};

#endif // LOGICNOT_H