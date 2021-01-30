/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICJKFLIPFLOP_H
#define LOGICJKFLIPFLOP_H

#include "logicelement.h"

class LogicJKFlipFlop : public LogicElement
{
public:
    explicit LogicJKFlipFlop();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;

private:
    bool lastClk;
    bool lastJ;
    bool lastK;
};

#endif // LOGICJKFLIPFLOP_H