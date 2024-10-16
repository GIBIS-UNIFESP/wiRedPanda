// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicMux : public LogicElement
{
public:
    explicit LogicMux();

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicMux)
};
