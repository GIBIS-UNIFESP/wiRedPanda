// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicInput : public LogicElement
{
public:
    explicit LogicInput(const bool defaultValue = false, const int nOutputs = 1);

protected:
    void _updateLogic(const QVector<bool> &inputs) override;
};
