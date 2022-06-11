// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicelement.h"

LogicElement::LogicElement(const int inputSize, const int outputSize)
    : m_inputs(inputSize, {})
    , m_inputValues(inputSize, false)
    , m_outputs(outputSize, false)
{
}

bool LogicElement::isValid() const
{
    return m_isValid;
}

void LogicElement::clearPredecessors()
{
    std::fill(m_inputs.begin(), m_inputs.end(), InputPair{});
}

void LogicElement::clearSucessors()
{
    for (const auto &elm : qAsConst(m_successors)) {
        for (auto &input : elm->m_inputs) {
            if (input.elm == this) {
                input.elm = nullptr;
                input.port = 0;
            }
        }
    }
    m_successors.clear();
}

void LogicElement::updateLogic()
{
    if (m_isValid) {
        for (int index = 0; index < m_inputs.size(); ++index) {
            m_inputValues[index] = inputValue(index);
        }
        _updateLogic(m_inputValues);
    }
}

void LogicElement::connectPredecessor(const int index, LogicElement *elm, const int port)
{
    m_inputs[index] = {elm, port};
    elm->m_successors.insert(this);
}

void LogicElement::setOutputValue(const int index, const bool value)
{
    m_outputs[index] = value;
}

void LogicElement::setOutputValue(const bool value)
{
    m_outputs[0] = value;
}

void LogicElement::validate()
{
    m_isValid = true;

    for (int index = 0; index < m_inputs.size() && m_isValid; ++index) {
        if (m_inputs[index].elm == nullptr) {
            m_isValid = false;
        }
    }

    if (!m_isValid) {
        for (auto *elm : qAsConst(m_successors)) {
            elm->m_isValid = false;
        }
    }
}

bool LogicElement::operator<(const LogicElement &other) const
{
    return m_priority < other.m_priority;
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
    for (auto *s : qAsConst(m_successors)) {
        max = qMax(s->calculatePriority(), max);
    }
    const int p = max + 1;
    m_priority = p;
    m_beingVisited = false;
    return p;
}

bool LogicElement::outputValue(const int index) const
{
    return m_outputs.at(index);
}

bool LogicElement::inputValue(const int index) const
{
    LogicElement *pred = m_inputs[index].elm;
    int port = m_inputs[index].port;
    return pred->outputValue(port);
}
