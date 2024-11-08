// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicjkflipflop.h"

LogicJKFlipFlop::LogicJKFlipFlop()
    : LogicElement(5, 2, 18)
{
    setOutputValue(0, false);
    setOutputValue(1, true);
}

void LogicJKFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }

    if (isTempSimulationOn()) {
        bool q0 = outputValue(0);
        bool q1 = outputValue(1);
        const bool j = m_inputBuffer.last()[0];
        const bool clk = m_inputBuffer.last()[1];
        const bool k = m_inputBuffer.last()[2];
        const bool prst = m_inputBuffer.last()[3];
        const bool clr = m_inputBuffer.last()[4];

        if (clk && !m_lastClk) {
            if (m_lastJ && m_lastK) {
                std::swap(q0, q1);
            } else if (m_lastJ) {
                q0 = true;
                q1 = false;
            } else if (m_lastK) {
                q0 = false;
                q1 = true;
            }
        }

        if (!prst || !clr) {
            q0 = !prst;
            q1 = !clr;
        }

        m_lastClk = clk;
        m_lastK = k;
        m_lastJ = j;

        setOutputValue(0, q0);
        setOutputValue(1, q1);
        updateInputBuffer();
    } else {
        bool q0 = outputValue(0);
        bool q1 = outputValue(1);
        const bool j = m_inputValues.at(0);
        const bool clk = m_inputValues.at(1);
        const bool k = m_inputValues.at(2);
        const bool prst = m_inputValues.at(3);
        const bool clr = m_inputValues.at(4);

        if (clk && !m_lastClk) {
            if (m_lastJ && m_lastK) {
                std::swap(q0, q1);
            } else if (m_lastJ) {
                q0 = true;
                q1 = false;
            } else if (m_lastK) {
                q0 = false;
                q1 = true;
            }
        }

        if (!prst || !clr) {
            q0 = !prst;
            q1 = !clr;
        }

        m_lastClk = clk;
        m_lastK = k;
        m_lastJ = j;

        setOutputValue(0, q0);
        setOutputValue(1, q1);
    }
}
