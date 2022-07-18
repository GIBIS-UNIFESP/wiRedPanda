// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSet>
#include <QVector>

class LogicElement;

struct InputPair {
    LogicElement *logic = nullptr;
    int port = 0;
};

//! Represent logic elements in the simulation layer
class LogicElement
{
public:
    explicit LogicElement(const int inputSize, const int outputSize);
    virtual ~LogicElement() = default;
    bool operator<(const LogicElement &other) const;

    bool inputValue(const int index = 0) const;
    bool isValid() const;
    bool outputValue(const int index = 0) const;
    int calculatePriority();
    void clearPredecessors();
    void clearSucessors();
    void connectPredecessor(const int index, LogicElement *logic, const int port);
    void setOutputValue(const bool value);
    void setOutputValue(const int index, const bool value);
    void validate();

    // Secure call to _updateLogic() with current inputs.
    void updateLogic();

private:
    Q_DISABLE_COPY(LogicElement)

    // Main function to update the logic of an element. Computes the outputs, given the inputs
    virtual void _updateLogic(const QVector<bool> &inputs) = 0;

    QSet<LogicElement *> m_successors;
    QVector<InputPair> m_inputPairs;
    QVector<bool> m_inputValues;
    QVector<bool> m_outputValues;
    bool m_beingVisited = false;
    bool m_isValid = true;
    int m_priority = -1;
};
