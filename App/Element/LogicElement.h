// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Base class for all simulation-layer logic elements.
 */

#pragma once

#include <QVector>

class LogicElement;

/**
 * \struct InputPair
 * \brief Links an input slot of one LogicElement to an output port of another.
 */
struct InputPair
{
    LogicElement *logic = nullptr; ///< Predecessor logic element.
    int port = 0;                  ///< Output port index on the predecessor.
};

/**
 * \class LogicElement
 * \brief Represent logic elements in the simulation layer.
 */
class LogicElement
{
public:
    explicit LogicElement(const int inputSize, const int outputSize);

    virtual ~LogicElement() = default;

    // --- Validity ---

    bool isValid() const;
    void validate();

    // --- I/O Values ---

    bool inputValue(const int index = 0) const;
    bool outputValue(const int index = 0) const;
    int outputSize() const;

    // --- Logic ---

    const QVector<InputPair> &inputPairs() const;
    virtual void updateLogic() = 0;

    // --- Connection management ---

    void connectPredecessor(const int index, LogicElement *logic, const int port);

    // --- Output setting ---

    void setOutputValue(const bool value);
    void setOutputValue(const int index, const bool value);

protected:
    bool updateInputs();

    /// Read-only view of the cached input values for use by subclass updateLogic().
    const QVector<bool> &inputs() const { return m_inputValues; }

private:
    Q_DISABLE_COPY(LogicElement)

    // --- Members ---

    QVector<InputPair> m_inputPairs;
    QVector<bool> m_inputValues;
    QVector<bool> m_outputValues;
    bool m_isValid = true;
};

