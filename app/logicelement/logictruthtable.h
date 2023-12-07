// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"
#include <QBitArray>
class LogicTruthTable : public LogicElement
{
public:
    explicit LogicTruthTable(const int inputSize, const QBitArray& key);

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicTruthTable)
    QBitArray proposition;
};
