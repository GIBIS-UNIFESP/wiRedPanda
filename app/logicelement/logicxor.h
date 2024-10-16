
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicXor : public LogicElement
{
public:
    explicit LogicXor(const int inputSize);

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicXor)
};
