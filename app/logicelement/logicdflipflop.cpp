// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicdflipflop.h"

LogicDFlipFlop::LogicDFlipFlop()
    : LogicElement(4, 2, 13)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicDFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (isTempSimulationOn()) {
        bool q0 = outputValue(0);
        bool q1 = outputValue(1);
        const bool D = m_inputValues.at(0);
        const bool clk = m_inputValues.at(1);
        const bool prst = m_inputValues.at(2);
        const bool clr = m_inputValues.at(3);

        if (clk && !m_lastClk) {
            q0 = m_lastValue;
            q1 = !m_lastValue;
        }

        if (!prst || !clr) {
            q0 = !prst;
            q1 = !clr;
        }

        m_lastClk = clk;
        m_lastValue = D;

        setOutputValue(0, q0);
        setOutputValue(1, q1);
        updateInputBuffer();
    } else {
        bool q0 = outputValue(0);
        bool q1 = outputValue(1);
        const bool D = m_inputValues.at(0);
        const bool clk = m_inputValues.at(1);
        const bool prst = m_inputValues.at(2);
        const bool clr = m_inputValues.at(3);

        if (clk && !m_lastClk) {
            q0 = m_lastValue;
            q1 = !m_lastValue;
        }

        if (!prst || !clr) {
            q0 = !prst;
            q1 = !clr;
        }

        m_lastClk = clk;
        m_lastValue = D;

        setOutputValue(0, q0);
        setOutputValue(1, q1);
    }
}
