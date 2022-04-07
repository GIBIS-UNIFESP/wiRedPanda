/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicelement.h"

class LogicOutput : public LogicElement
{
public:
    explicit LogicOutput(size_t inputSz);

protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

