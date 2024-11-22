// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
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
    explicit LogicElement(const int inputSize, const int outputSize, int delayLength);
    virtual ~LogicElement() = default;
    bool operator>(const LogicElement &other) const;

    bool inputValue(const int index = 0) const;

    /**
     * Checks if Temporal Simulation is activated.
     * @return True if Temporal Simulation is activated, and False if it's not activated.
    */
    bool isTempSimulationOn();
    bool isValid() const;
    bool outputValue(const int index = 0) const;
    int calculatePriority();
    virtual void updateLogic() = 0;
    void clearSucessors();
    void connectPredecessor(const int index, LogicElement *logic, const int port);
    void setOutputValue(const bool value);
    void setOutputValue(const int index, const bool value);

    /**
     * Set the m_TempSimulationIsOn to True or False.
     * @param isOn is used to to knows which value will be used.
    */
    void setTemporalSimulationIsOn(bool isOn);

    /**
     * Propagates the values in m_inputBuffer in each line to the next line,
     * then needs delayLenght times to propagate the first line to final line,
     * simulating the delay an each logic port.
    */
    void updateInputBuffer();
    void validate();

protected:
    bool updateInputs();

    QVector<QVector<bool>> m_inputBuffer;
    QVector<bool> m_inputValues;

private:
    Q_DISABLE_COPY(LogicElement)

    QSet<LogicElement *> m_successors;
    QVector<InputPair> m_inputPairs;
    QVector<bool> m_outputValues;
    bool m_beingVisited = false;
    bool m_isValid = true;
    bool m_TempSimulationIsOn = false;
    int m_priority = -1;
    int m_delayCount = 0;
};
