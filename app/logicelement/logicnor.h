/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICNOR_H
#define LOGICNOR_H

#include "logicelement.h"

class LogicNor : public LogicElement
{
public:
    explicit LogicNor(size_t inputSize);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};
#endif // LOGICNOR_H