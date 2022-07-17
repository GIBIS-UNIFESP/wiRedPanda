// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicJKFlipFlop : public LogicElement
{
public:
    explicit LogicJKFlipFlop();

protected:
    void _updateLogic(const QVector<bool> &inputs) override;

private:
    bool m_lastClk = false;
    bool m_lastJ = true;
    bool m_lastK = true;
};
