/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicelement.h"

class LogicInput : public LogicElement
{
public:
    explicit LogicInput(const bool defaultValue = false, const int n_outputs = 1);

protected:
    void _updateLogic(const QVector<bool> &inputs) override;
};
