// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QVector>

class LogicElement;

struct InputPair
{
    LogicElement *logic = nullptr;
    int port = 0;
};

/*!
 * @class LogicElement
 * @brief Represent logic elements in the simulation layer
 */
class LogicElement
{
public:
    // --- Lifecycle ---

    explicit LogicElement(const int inputSize, const int outputSize);
    virtual ~LogicElement() = default;

    // --- Validity & Feedback ---

    bool isValid() const;
    bool inFeedbackLoop() const;
    void validate();

    // --- I/O Values ---

    bool inputValue(const int index = 0) const;
    bool outputValue(const int index = 0) const;
    int outputSize() const;

    // --- Priority & Logic ---

    int calculatePriority();
    int priority() const;
    virtual void updateLogic() = 0;

    // --- Connection management ---

    void connectPredecessor(const int index, LogicElement *logic, const int port);
    void clearSucessors();

    // --- Output setting ---

    void setOutputValue(const bool value);
    void setOutputValue(const int index, const bool value);

protected:
    // --- Internal updates ---

    bool updateInputs();

    QVector<bool> m_inputValues;

private:
    Q_DISABLE_COPY(LogicElement)

    // --- Members ---

    QVector<LogicElement *> m_successors;
    QVector<InputPair> m_inputPairs;
    QVector<bool> m_outputValues;
    bool m_inFeedbackLoop = false;
    bool m_isValid = true;
    int m_priority = -1;
};
