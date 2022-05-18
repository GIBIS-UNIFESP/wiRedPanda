/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QSet>
#include <QVector>

class LogicElement;

struct InputPair {
    LogicElement *elm = nullptr;
    int port = 0;
};

/**
 * @brief The LogicElement class was designed to represent logic
 *        elements in the simulation layer of WiRedPanda.
 */
class LogicElement
{
public:
    explicit LogicElement(const int inputSize, const int outputSize);
    virtual ~LogicElement();

    bool inputValue(const int index = 0) const;
    bool outputValue(const int index = 0) const;
    bool isValid() const;
    bool operator<(const LogicElement &other) const;
    int calculatePriority();
    void clearPredecessors();
    void clearSucessors();
    void connectPredecessor(const int index, LogicElement *elm, const int port);
    void setOutputValue(const bool value);
    void setOutputValue(const int index, const bool value);
    void validate();

    // Secure call to _updateLogic() with current inputs.
    void updateLogic();

protected:
    // Main function to update the logic of an element. Computes the outputs, given the inputs
    virtual void _updateLogic(const QVector<bool> &inputs) = 0;

private:
    QSet<LogicElement *> m_successors;
    QVector<InputPair> m_inputs;
    QVector<bool> m_inputvalues;
    QVector<bool> m_outputs;
    bool m_beingVisited = false;
    bool m_isValid = true;
    int m_priority = -1;
};

