// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QVector>

class LogicElement;

struct InputPair
{
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

    bool inputValue(const int index = 0) const;
    bool isTempSimulationOn();
    bool isValid() const;
    bool outputValue(const int index = 0) const;
    int calculatePriority();
    int priority() const;
    virtual void updateLogic() = 0;
    void clearSucessors();
    void connectPredecessor(const int index, LogicElement *logic, const int port);
    void setOutputValue(const bool value);
    void setOutputValue(const int index, const bool value);
    void setTemporalSimulationIsOn(bool isOn);
    void updateInputBuffer();
    void validate();

protected:
    bool updateInputs();

    QVector<QVector<bool>> m_inputBuffer;
    QVector<bool> m_inputValues;
    int m_delayLength;

private:
    Q_DISABLE_COPY(LogicElement)

    void updateInputsInBuffer();

    QVector<LogicElement *> m_successors;
    QVector<InputPair> m_inputPairs;
    QVector<bool> m_outputValues;
    bool m_beingVisited = false;
    bool m_isValid = true;
    bool m_tempSimulationIsOn = false;
    int m_priority = -1;
};
