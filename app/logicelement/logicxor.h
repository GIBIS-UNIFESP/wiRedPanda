#ifndef LOGICXOR_H
#define LOGICXOR_H

#include "logicelement.h"

class LogicXor : public LogicElement
{
public:
    explicit LogicXor(size_t inputSize);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICXOR_H