// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicelement.h"

LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_inputPairs(inputSize, {})
    , m_inputValues(inputSize, false)
    , m_outputValues(outputSize, false)
{
}

bool LogicElement::isValid() const
{
    return m_isValid;
}

void LogicElement::clearPredecessors()
{
    std::fill(m_inputPairs.begin(), m_inputPairs.end(), InputPair{});
}

void LogicElement::clearSucessors()
{
    for (const auto &logic : qAsConst(m_successors)) {
        for (auto &inputPair : logic->m_inputPairs) {
            if (inputPair.logic == this) {
                inputPair.logic = nullptr;
                inputPair.port = 0;
            }
        }
    }

    m_successors.clear();
}

void LogicElement::updateLogic()
{
    if (!m_isValid) {
        return;
    }

    for (int index = 0; index < m_inputPairs.size(); ++index) {
        m_inputValues[index] = inputValue(index);
    }

    _updateLogic(m_inputValues);
}

void LogicElement::connectPredecessor(const int index, LogicElement *logic, const int port)
{
    m_inputPairs[index] = {logic, port};
    logic->m_successors.insert(this);
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
    m_isValid = true;

    for (int index = 0; index < m_inputPairs.size() && m_isValid; ++index) {
        if (m_inputPairs[index].logic == nullptr) {
            m_isValid = false;
        }
    }

    if (!m_isValid) {
        for (auto *logic : qAsConst(m_successors)) {
            logic->m_isValid = false;
        }
    }
}

bool LogicElement::operator<(const LogicElement &other) const
{
    return (m_priority < other.m_priority);
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

    for (auto *logic : qAsConst(m_successors)) {
        max = qMax(logic->calculatePriority(), max);
    }

    const int priority = max + 1;
    m_priority = priority;
    m_beingVisited = false;
    return priority;
}

bool LogicElement::outputValue(const int index) const
{
    return m_outputValues.at(index);
}

bool LogicElement::inputValue(const int index) const
{
    auto *pred = m_inputPairs[index].logic;
    int port = m_inputPairs[index].port;
    return pred->outputValue(port);
}
