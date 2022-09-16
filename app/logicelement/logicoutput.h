// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicOutput : public LogicElement
{
public:
    explicit LogicOutput(const int inputSize);

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicOutput)
};
