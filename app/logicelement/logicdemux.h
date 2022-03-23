/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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