// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/LogicElements/LogicElement.h"

class LogicOr : public LogicElement
{
public:
    explicit LogicOr(const int inputSize);

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicOr)
};
