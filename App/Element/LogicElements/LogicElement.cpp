// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicElement.h"

LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_inputPairs(inputSize, {})
    , m_inputValues(inputSize, Status::Inactive)
    , m_outputValues(outputSize, Status::Inactive)
{
}

bool LogicElement::updateInputs()
{
    // Snapshot predecessor outputs into m_inputValues so that updateLogic()
    // can read a consistent view even when predecessor values change mid-cycle.
    for (int index = 0; index < m_inputPairs.size(); ++index) {
        const Status val = inputValue(index);
        if (val == Status::Invalid) {
            // An unconnected (null) predecessor makes all outputs Invalid; the
            // simulation cannot compute a meaningful result from incomplete data.
            for (auto &out : m_outputValues) {
                if (out != Status::Invalid) {
                    m_outputChanged = true;
                }
                out = Status::Invalid;
            }
            return false;
        }
        m_inputValues[index] = val;
    }

    return true;
}

int LogicElement::outputSize() const
{
    return m_outputValues.size();
}

const QVector<InputPair> &LogicElement::inputPairs() const
{
    return m_inputPairs;
}

void LogicElement::connectPredecessor(const int index, LogicElement *logic, const int port)
{
    m_inputPairs[index] = {logic, port};
}

void LogicElement::setOutputValue(const int index, const Status value)
{
    if (m_outputValues[index] != value) {
        m_outputChanged = true;
    }
    m_outputValues[index] = value;
}

void LogicElement::setOutputValue(const Status value)
{
    setOutputValue(0, value);
}

Status LogicElement::outputValue(const int index) const
{
    return m_outputValues.at(index);
}

Status LogicElement::inputValue(const int index) const
{
    // Traverse one edge of the logic graph: read the output of the predecessor
    // element at the specific port that drives this input slot.
    // A null predecessor (unconnected input port) returns Invalid.
    auto *pred = m_inputPairs.at(index).logic;
    if (!pred) {
        return Status::Invalid;
    }
    int port = m_inputPairs.at(index).port;
    return pred->outputValue(port);
}
