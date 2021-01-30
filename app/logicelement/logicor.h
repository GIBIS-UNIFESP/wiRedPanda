#ifndef LOGICOR_H
#define LOGICOR_H

#include "logicelement.h"

class LogicOr : public LogicElement
{
public:
    explicit LogicOr(size_t inputSize);

    /* LogicElement interface */
protected:
    virtual void _updateLogic(const std::vector<bool> &inputs);
};

#endif // LOGICOR_H