// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicNone : public LogicElement
{
public:
    explicit LogicNone();

private:
    Q_DISABLE_COPY(LogicNone)

    void updateLogic() override;
    void notifyNextElement();
};
