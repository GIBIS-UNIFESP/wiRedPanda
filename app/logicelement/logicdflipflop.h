/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICDFLIPFLOP_H
#define LOGICDFLIPFLOP_H

#include "logicelement.h"

class LogicDFlipFlop : public LogicElement
{
public:
    explicit LogicDFlipFlop();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;

private:
    bool lastClk;
    bool lastValue;
};

#endif // LOGICDFLIPFLOP_H
