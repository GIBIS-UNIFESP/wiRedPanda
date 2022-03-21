/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICNAND_H
#define LOGICNAND_H

#include "logicelement.h"

class LogicNand : public LogicElement
{
public:
    explicit LogicNand(size_t inputSize);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICNAND_H