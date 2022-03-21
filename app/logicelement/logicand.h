/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICAND_H
#define LOGICAND_H

#include "logicelement.h"

class LogicAnd : public LogicElement
{
public:
    explicit LogicAnd(size_t inputSize);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICAND_H
