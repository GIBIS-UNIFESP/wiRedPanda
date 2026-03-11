// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QBitArray>

#include "App/Element/LogicElements/LogicElement.h"

class LogicTruthTable : public LogicElement
{
public:
    // --- Lifecycle ---

    explicit LogicTruthTable(const int inputSize, const int outputSize, const QBitArray &key);

    // --- Logic ---

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicTruthTable)

    // --- Members ---

    QBitArray m_proposition;
    int m_nOutputs;
};
