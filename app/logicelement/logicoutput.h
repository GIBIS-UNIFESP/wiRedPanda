/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICOUTPUT_H
#define LOGICOUTPUT_H

#include "logicelement.h"

class LogicOutput : public LogicElement
{
public:
    explicit LogicOutput(size_t inputSz);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICOUTPUT_H