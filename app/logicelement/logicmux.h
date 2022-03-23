/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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