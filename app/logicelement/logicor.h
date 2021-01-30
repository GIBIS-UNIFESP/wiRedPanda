#ifndef LOGICOR_H
#define LOGICOR_H

#include "logicelement.h"

class LogicOr : public LogicElement
{
public:
    explicit LogicOr(size_t inputSize);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICOR_H