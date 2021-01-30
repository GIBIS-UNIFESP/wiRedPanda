#ifndef LOGICAND_H
#define LOGICAND_H

#include "logicelement.h"

class LogicAnd : public LogicElement
{
public:
    explicit LogicAnd(size_t inputSize);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICAND_H