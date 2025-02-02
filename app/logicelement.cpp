// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicelement.h"

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

void LogicElement::clearSucessors()
{
    for (const auto &logic : std::as_const(m_successors)) {
        for (auto &inputPair : logic->m_inputPairs) {
            if (inputPair.logic == this) {
                inputPair.logic = nullptr;
                inputPair.port = 0;
            }
        }
    }

    m_successors.clear();
}

bool LogicElement::updateInputs()
{
    if (!m_isValid) {
        return false;
    }

    for (int index = 0; index < m_inputPairs.size(); ++index) {
        m_inputValues[index] = inputValue(index);
    }

    return true;
}

int LogicElement::priority() const
{
    return m_priority;
}

void LogicElement::connectPredecessor(const int index, LogicElement *logic, const int port)
{
    m_inputPairs[index] = {logic, port};

    if (!logic->m_successors.contains(this)) {
        logic->m_successors.push_back(this);
    }
}

void LogicElement::setOutputValue(const int index, const bool value)
{
    m_outputValues[index] = value;
}

void LogicElement::setOutputValue(const bool value)
{
    setOutputValue(0, value);
}

void LogicElement::validate()
{
    m_isValid = std::all_of(m_inputPairs.cbegin(), m_inputPairs.cend(),
                            [](auto pair) { return pair.logic != nullptr; });

    if (!m_isValid) {
        for (auto *logic : std::as_const(m_successors)) {
            logic->m_isValid = false;
        }
    }
}

int LogicElement::calculatePriority()
{
    if (m_beingVisited) {
        return 0;
    }

    if (m_priority != -1) {
        return m_priority;
    }

    m_beingVisited = true;
    int max = 0;

    for (auto *logic : std::as_const(m_successors)) {
        max = qMax(logic->calculatePriority(), max);
    }

    m_beingVisited = false;
    m_priority = max + 1;
    return m_priority;
}

bool LogicElement::outputValue(const int index) const
{
    return m_outputValues.at(index);
}

bool LogicElement::inputValue(const int index) const
{
    auto *pred = m_inputPairs.at(index).logic;
    int port = m_inputPairs.at(index).port;
    return pred->outputValue(port);
}
