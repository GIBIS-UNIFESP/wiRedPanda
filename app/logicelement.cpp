// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicelement.h"

#include <QHash>
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

bool LogicElement::inFeedbackLoop() const
{
    return m_inFeedbackLoop;
}

int LogicElement::outputSize() const
{
    return m_outputValues.size();
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
    QStack<LogicElement *> stack;
    QHash<LogicElement *, bool> inStack;
    QHash<LogicElement *, int> maxPriority;
    QHash<LogicElement *, bool> inFeedbackLoop;

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

        for (auto *logic : std::as_const(current->m_successors)) {
            if (logic->m_priority == -1) {
                if (!inStack[logic]) {
                    stack.push(logic);
                    inStack[logic] = true;
                    allProcessed = false;
                } else {
                    // FEEDBACK LOOP DETECTED: successor is already in processing stack
                    hasFeedbackLoop = true;
                    inFeedbackLoop[current] = true;
                    inFeedbackLoop[logic] = true;
                    // For feedback loops, don't wait for successor priority
                    // This breaks the circular dependency
                }
            }

            // Only use successor priority if it's not a feedback loop
            if (logic->m_priority != -1) {
                maxSuccessorPriority = qMax(maxSuccessorPriority, logic->m_priority);
            }
        }

        if (allProcessed || hasFeedbackLoop) {
            if (hasFeedbackLoop) {
                // Elements in feedback loops get special priority handling
                // Set to a base priority that allows for iterative settling
                current->m_priority = maxSuccessorPriority + 1;
                current->m_inFeedbackLoop = true;
            } else {
                current->m_priority = maxSuccessorPriority + 1;
                current->m_inFeedbackLoop = false;
            }
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
    if (!pred) {
        return false;
    }
    int port = m_inputPairs.at(index).port;
    return pred->outputValue(port);
}
