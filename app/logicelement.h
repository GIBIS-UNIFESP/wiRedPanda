// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
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
    bool isValid() const;
    bool isTempSimulationOn();
    bool outputValue(const int index = 0) const;
    int calculatePriority();
    virtual void updateLogic() = 0;
    void clearSucessors();
    void connectPredecessor(const int index, LogicElement *logic, const int port);
    void setOutputValue(const bool value);
    void setOutputValue(const int index, const bool value);
    void setTemporalSimulationIsOn(bool isOn);
    void validate();

    void updateInputBuffer();
    //MEXI AQUI, MEXI NO NOT, NO AND E NO SIMULATION

protected:
    bool updateInputs();

    QVector<bool> m_inputValues;
    QVector<QVector<bool>> inputBuffer;
    int m_delayLength;

private:
    Q_DISABLE_COPY(LogicElement)

    QSet<LogicElement *> m_successors;
    QVector<InputPair> m_inputPairs;
    QVector<bool> m_outputValues;
    bool m_beingVisited = false;
    bool m_isValid = true;
    bool m_TempSimulationIsOn = false;
    int m_priority = -1;
};
