// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicelement.h"

#include <QDebug>
#include <QHash>
#include <QStack>

LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_inputValues(inputSize, false)
    , m_inputPairs(inputSize, {})
    , m_outputValues(outputSize, false)
{
    qDebug() << "LOGIC ELEMENT DEBUG: Creating LogicElement" << (void*)this << "with" << inputSize << "inputs," << outputSize << "outputs";
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
    qDebug() << "CONNECT PREDECESSOR DEBUG: Element" << (void*)this << "input" << index 
             << "connecting to predecessor" << (void*)logic << "port" << port;
    qDebug() << "CONNECT PREDECESSOR DEBUG: Before - Input" << index << "was" << (void*)m_inputPairs[index].logic << "port" << m_inputPairs[index].port;
    
    m_inputPairs[index] = {logic, port};
    
    qDebug() << "CONNECT PREDECESSOR DEBUG: After - Input" << index << "now" << (void*)m_inputPairs[index].logic << "port" << m_inputPairs[index].port;

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

    // Debug validation failures
    if (!m_isValid) {
        qDebug() << "LOGIC ELEMENT DEBUG: Element invalid - checking input pairs:";
        for (int i = 0; i < m_inputPairs.size(); ++i) {
            qDebug() << "  Input" << i << ":" << (void*)m_inputPairs[i].logic << "port" << m_inputPairs[i].port;
        }
        
        for (auto *logic : std::as_const(m_successors)) {
            logic->m_isValid = false;
        }
    }
}

int LogicElement::calculatePriority()
{
    QStack<LogicElement *> stack;
    QHash<LogicElement *, bool> inStack;
    QHash<LogicElement *, int> maxPriority;

    stack.push(this);
    inStack[this] = true;

    while (!stack.isEmpty()) {
        auto *current = stack.top();

        if (current->m_priority != -1) {
            stack.pop();
            inStack[current] = false;
            continue;
        }

        bool allProcessed = true;
        int maxSuccessorPriority = 0;

        for (auto *logic : std::as_const(current->m_successors)) {
            if (logic->m_priority == -1) {
                if (!inStack[logic]) {
                    stack.push(logic);
                    inStack[logic] = true;
                    allProcessed = false;
                }
            }

            maxSuccessorPriority = qMax(maxSuccessorPriority, logic->m_priority);
        }

        if (allProcessed) {
            current->m_priority = maxSuccessorPriority + 1;
            stack.pop();
            inStack[current] = false;
        }
    }

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

QVector<bool> LogicElement::getAllOutputValues() const
{
    return m_outputValues;
}

bool LogicElement::hasOutputChanged(const QVector<bool> &previousOutputs) const
{
    return m_outputValues != previousOutputs;
}

void LogicElement::setFeedbackDependent(bool isFeedbackDependent)
{
    m_isFeedbackDependent = isFeedbackDependent;
}

bool LogicElement::isFeedbackDependent() const
{
    return m_isFeedbackDependent;
}

int LogicElement::inputCount() const
{
    return m_inputPairs.size();
}

LogicElement* LogicElement::getInputSource(int index) const
{
    if (index >= 0 && index < m_inputPairs.size()) {
        return m_inputPairs[index].logic;
    }
    return nullptr;
}
