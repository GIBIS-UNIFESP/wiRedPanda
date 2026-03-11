// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LogicElement.h"

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

void LogicElement::clearSucessors()
{
    // Walk every successor and null-out any InputPair that still points back
    // to this element, so dangling pointers can't be followed after deletion.
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

void LogicElement::connectPredecessor(const int index, LogicElement *logic, const int port)
{
    m_inputPairs[index] = {logic, port};

    // Register this element as a successor of the predecessor so that
    // clearSuccessors() and priority propagation can traverse downstream edges.
    // The duplicate guard prevents a second registration when multiple input
    // slots share the same predecessor element.
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
    // An element is valid only when every input slot has a predecessor logic
    // node; unconnected required inputs leave a null pointer and mark the
    // element invalid so its outputs propagate Status::Invalid downstream.
    m_isValid = std::all_of(m_inputPairs.cbegin(), m_inputPairs.cend(),
                            [](auto pair) { return pair.logic != nullptr; });

    if (!m_isValid) {
        // Propagate invalidity one level downstream; the full transitive closure
        // is reached because each successor calls validate() in turn during the
        // same simulation-setup pass.
        for (auto *logic : std::as_const(m_successors)) {
            logic->m_isValid = false;
        }
    }
}

int LogicElement::calculatePriority()
{
    // Iterative post-order DFS that computes a topological depth for each node.
    // Higher priority means the element is deeper in the DAG and must be updated
    // later (after all its predecessors) during each simulation cycle.
    // m_priority == -1 serves as "not yet assigned" sentinel (see header).
    QStack<LogicElement *> stack;
    QMap<LogicElement *, bool> inStack;
    QMap<LogicElement *, int> maxPriority;   // unused bookkeeping — kept for clarity
    QMap<LogicElement *, bool> inFeedbackLoop;

    stack.push(this);
    inStack[this] = true;

    while (!stack.isEmpty()) {
        auto *current = stack.top();

        // Already assigned on a previous visit (e.g. shared successor); skip.
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
                    // Successor not yet visited — push it and process first (DFS).
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
                // Normal case: priority is one greater than the deepest successor,
                // ensuring this element is updated before any of its dependents.
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
    // Traverse one edge of the logic graph: read the output of the predecessor
    // element at the specific port that drives this input slot.
    auto *pred = m_inputPairs.at(index).logic;
    if (!pred) {
        return false;
    }
    int port = m_inputPairs.at(index).port;
    return pred->outputValue(port);
}

