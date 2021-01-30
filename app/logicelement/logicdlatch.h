#ifndef LOGICDLATCH_H
#define LOGICDLATCH_H

#include "logicelement.h"

class LogicDLatch : public LogicElement
{
public:
    explicit LogicDLatch();

    /* LogicElement interface */
protected:
    virtual void _updateLogic(const std::vector<bool> &inputs);
};

#endif // LOGICDLATCH_H