
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicNand : public LogicElement
{
public:
    explicit LogicNand(const int inputSize);

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicNand)
};
