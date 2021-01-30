#ifndef LOGICMUX_H
#define LOGICMUX_H

#include "logicelement.h"

class LogicMux : public LogicElement
{
public:
    explicit LogicMux();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICMUX_H