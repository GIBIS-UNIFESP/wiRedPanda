// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicMux : public LogicElement
{
public:
    explicit LogicMux();

protected:
    void _updateLogic(const QVector<bool> &inputs) override;
};
