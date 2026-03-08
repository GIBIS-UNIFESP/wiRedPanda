// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElements/LogicElement.h"

#include <QHash>
#include <QMap>
#include <QStack>

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
                            [](const auto &pair) { return pair.logic != nullptr; });
}

int LogicElement::calculatePriority(const QHash<LogicElement *, QVector<LogicElement *>> &successors)
{
    QStack<LogicElement *> stack;
    QMap<LogicElement *, bool> inStack;

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
        bool hasFeedbackLoop = false;

        const auto it = successors.find(current);
        if (it != successors.end()) {
            for (auto *logic : *it) {
                if (logic->m_priority == -1) {
                    if (!inStack[logic]) {
                        stack.push(logic);
                        inStack[logic] = true;
                        allProcessed = false;
                    } else {
                        hasFeedbackLoop = true;
                    }
                }

                if (logic->m_priority != -1) {
                    maxSuccessorPriority = qMax(maxSuccessorPriority, logic->m_priority);
                }
            }
        }

        if (allProcessed || hasFeedbackLoop) {
            current->m_priority = maxSuccessorPriority + 1;
            current->m_inFeedbackLoop = hasFeedbackLoop;
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
    // Traverse one edge of the logic graph: read the output of the predecessor
    // element at the specific port that drives this input slot.
    auto *pred = m_inputPairs.at(index).logic;
    if (!pred) {
        return false;
    }
    int port = m_inputPairs.at(index).port;
    return pred->outputValue(port);
}
