// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElement.h"

LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_inputValues(inputSize, false)
    , m_inputPairs(inputSize, {})
    , m_outputValues(outputSize, false)
{
}

bool LogicElement::isValid() const
{
    return m_isValid;
}

bool LogicElement::updateInputs()
{
    if (!m_isValid) {
        return false;
    }

    // Snapshot predecessor outputs into m_inputValues so that updateLogic()
    // can read a consistent view even when predecessor values change mid-cycle.
    for (int index = 0; index < m_inputPairs.size(); ++index) {
        m_inputValues[index] = inputValue(index);
    }

    return true;
}

int LogicElement::priority() const
{
    return m_priority;
}

bool LogicElement::inFeedbackLoop() const
{
    return m_inFeedbackLoop;
}

int LogicElement::outputSize() const
{
    return static_cast<int>(m_outputValues.size());
}

const QVector<InputPair> &LogicElement::inputPairs() const
{
    return m_inputPairs;
}

void LogicElement::connectPredecessor(const int index, LogicElement *logic, const int port)
{
    m_inputPairs[index] = {logic, port};
}

void LogicElement::setInFeedbackLoop(const bool inLoop)
{
    m_inFeedbackLoop = inLoop;
}

void LogicElement::setOutputValue(const int index, const bool value)
{
    m_outputValues[index] = value;
}

void LogicElement::setOutputValue(const bool value)
{
    setOutputValue(0, value);
}

void LogicElement::setPriority(const int priority)
{
    m_priority = priority;
}

void LogicElement::validate()
{
    m_isValid = std::all_of(m_inputPairs.cbegin(), m_inputPairs.cend(),
                            [](const auto &pair) { return pair.logic != nullptr; });
}

bool LogicElement::outputValue(const int index) const
{
    return m_outputValues.at(index);
}

bool LogicElement::inputValue(const int index) const
{
    // Traverse one edge of the logic graph: read the output of the predecessor
    // element at the specific port that drives this input slot.
    auto *pred = m_inputPairs.at(index).logic;
    if (!pred) {
        return false;
    }
    int port = m_inputPairs.at(index).port;
    return pred->outputValue(port);
}

