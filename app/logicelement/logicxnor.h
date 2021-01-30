#ifndef LOGICXNOR_H
#define LOGICXNOR_H

#include "logicelement.h"

class LogicXnor : public LogicElement
{
public:
    explicit LogicXnor(size_t inputSize);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICXNOR_H