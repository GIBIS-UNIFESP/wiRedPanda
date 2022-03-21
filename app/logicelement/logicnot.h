/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICNOT_H
#define LOGICNOT_H

#include "logicelement.h"

class LogicNot : public LogicElement
{
public:
    explicit LogicNot();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICNOT_H