/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicelement.h"

class LogicOr : public LogicElement
{
public:
    explicit LogicOr(const int inputSize);

protected:
    void _updateLogic(const QVector<bool> &inputs) override;

private:
    Q_DISABLE_COPY(LogicOr)
};
