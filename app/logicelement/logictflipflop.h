/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICTFLIPFLOP_H
#define LOGICTFLIPFLOP_H

#include "logicelement.h"

class LogicTFlipFlop : public LogicElement
{
public:
    explicit LogicTFlipFlop();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;

private:
    bool lastClk;
    bool lastValue;
};

#endif // LOGICTFLIPFLOP_H