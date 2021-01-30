#ifndef LOGICDEMUX_H
#define LOGICDEMUX_H

#include "logicelement.h"

class LogicDemux : public LogicElement
{
public:
    explicit LogicDemux();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICDEMUX_H