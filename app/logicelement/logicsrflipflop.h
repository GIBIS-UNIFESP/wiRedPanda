/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICSRFLIPFLOP_H
#define LOGICSRFLIPFLOP_H

#include "logicelement.h"

class LogicSRFlipFlop : public LogicElement
{
public:
    explicit LogicSRFlipFlop();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;

private:
    bool lastClk;
};

#endif // LOGICSRFLIPFLOP_H