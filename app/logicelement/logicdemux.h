// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

class LogicDemux : public LogicElement
{
public:
    explicit LogicDemux();

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicDemux)
};
