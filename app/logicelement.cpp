// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logicelement.h"

LogicElement::LogicElement(size_t inputSize, size_t outputSize)
    : m_isValid(true)
    , m_beingVisited(false)
    , m_priority(-1)
    , m_inputs(inputSize, std::make_pair(nullptr, 0))
    , m_inputvalues(inputSize, false)
    , m_outputs(outputSize, false)
{
}

LogicElement::~LogicElement() = default;

bool LogicElement::isValid() const
{
    return m_isValid;
}

void LogicElement::clearPredecessors()
{
    std::fill(m_inputs.begin(), m_inputs.end(), std::make_pair(nullptr, 0));
}

void LogicElement::clearSucessors()
{
    for (auto &elm : qAsConst(m_successors)) {
        for (auto &input : elm->m_inputs) {
            if (input.first == this) {
                input.first = nullptr;
                input.second = 0;
            }
        }
    }
    m_successors.clear();
}

void LogicElement::updateLogic()
{
    if (m_isValid) {
        for (size_t idx = 0; idx < m_inputs.size(); ++idx) {
            m_inputvalues[idx] = getInputValue(idx);
        }
        _updateLogic(m_inputvalues);
    }
}

void LogicElement::connectPredecessor(int index, LogicElement *elm, int port)
{
    m_inputs.at(index) = std::make_pair(elm, port);
    elm->m_successors.insert(this);
}

void LogicElement::setOutputValue(size_t index, bool value)
{
    m_outputs.at(index) = value;
}

void LogicElement::setOutputValue(bool value)
{
    m_outputs.at(0) = value;
}

void LogicElement::validate()
{
    m_isValid = true;
    for (size_t in = 0; in < m_inputs.size() && m_isValid; ++in) {
        if (m_inputs[in].first == nullptr) {
            m_isValid = false;
        }
    }
    if (!m_isValid) {
        for (LogicElement *elm : qAsConst(m_successors)) {
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
    for (LogicElement *s : qAsConst(m_successors)) {
        max = qMax(s->calculatePriority(), max);
    }
    const int p = max + 1;
    m_priority = p;
    m_beingVisited = false;
    return p;
}

bool LogicElement::getOutputValue(size_t index) const
{
    return m_outputs.at(index);
}

bool LogicElement::getInputValue(size_t index) const
{
    Q_ASSERT(m_isValid);
    LogicElement *pred = m_inputs[index].first;
    Q_ASSERT(pred);
    int port = m_inputs[index].second;
    return pred->getOutputValue(port);
}
