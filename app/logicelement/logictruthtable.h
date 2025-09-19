// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "logicelement.h"

#include <QBitArray>

class LogicTruthTable : public LogicElement
{
public:
    explicit LogicTruthTable(const int inputSize, const int outputSize, const QBitArray &key);

    void updateLogic() override;
    const QBitArray &proposition() const { return m_proposition; }

private:
    Q_DISABLE_COPY(LogicTruthTable)

    QBitArray m_proposition;
    int m_nOutputs;
};
