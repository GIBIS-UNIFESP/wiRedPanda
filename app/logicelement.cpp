// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicelement.h"

LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_inputValues(inputSize, false)
    , m_inputPairs(inputSize, {})
    , m_outputValues(outputSize, false)
{
}

LogicElement::LogicElement(const int inputSize, const int outputSize, int delayLength)
    : m_inputValues(inputSize, false)
    , m_inputPairs(inputSize, {})
    , m_outputValues(outputSize, false)
{
    m_inputBuffer = QVector<QVector<bool>>(delayLength);

    for (int i = 0; i < m_inputBuffer.length(); i++) {
        m_inputBuffer[i] = QVector<bool>(inputSize);
    }
}

bool LogicElement::isValid() const
{
    return m_isValid;
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
    m_isValid = std::all_of(m_inputPairs.cbegin(), m_inputPairs.cend(),
                            [](auto pair) { return pair.logic != nullptr; });

    if (!m_isValid) {
        for (auto *logic : qAsConst(m_successors)) {
            logic->m_isValid = false;
        }
    }
}

bool LogicElement::operator>(const LogicElement &other) const
{
    return (m_priority > other.m_priority);
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
    auto *pred = m_inputPairs.at(index).logic;
    int port = m_inputPairs.at(index).port;
    return pred->outputValue(port);
}

void LogicElement::updateInputBuffer() {
    for (int j = 0; j < m_inputValues.length(); j++) {
        m_inputBuffer[0][j] = inputValue(j);
        for (int i = 1; i < m_inputBuffer.length(); i++) {
            m_inputBuffer[i][j] = m_inputBuffer[i-1][j];
        }
    }
}

void LogicElement::setTemporalSimulationIsOn(bool isOn)
{
    m_TempSimulationIsOn = isOn;
    updateLogic();
}

bool LogicElement::isTempSimulationOn()
{
    return m_TempSimulationIsOn;
}
