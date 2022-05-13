/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicelement.h"

class LogicNone : public LogicElement
{
public:
    explicit LogicNone();

protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

