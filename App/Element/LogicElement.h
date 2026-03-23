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
    /**
     * \brief Constructs a logic element with \a inputSize inputs and \a outputSize outputs.
     * \param inputSize  Number of input slots.
     * \param outputSize Number of output slots.
     */
    explicit LogicElement(const int inputSize, const int outputSize);

    /// Virtual destructor.
    virtual ~LogicElement() = default;

    // --- Validity ---

    /// Returns \c true if all required input ports are connected.
    bool isValid() const;
    /// Recomputes and caches the validity state.
    void validate();

    // --- I/O Values ---

    /// Returns the current logic value on input port \a index.
    bool inputValue(const int index = 0) const;
    /// Returns the current logic value on output port \a index.
    bool outputValue(const int index = 0) const;
    /// Returns the number of output ports.
    int outputSize() const;

    // --- Change tracking ---

    /// Returns \c true if any output changed during the last updateLogic() call.
    bool outputChanged() const { return m_outputChanged; }
    /// Clears the output-changed flag (call before updateLogic() to prepare for change detection).
    void clearOutputChanged() { m_outputChanged = false; }

    // --- Logic ---

    const QVector<InputPair> &inputPairs() const;
    virtual void updateLogic() = 0;

    // --- Connection management ---

    /** \brief Connects input \a index of this element to output \a port of \a logic. */
    void connectPredecessor(const int index, LogicElement *logic, const int port);

    // --- Output setting ---

    /// Sets all output ports to \a value.
    void setOutputValue(const bool value);
    /// Sets output port \a index to \a value.
    void setOutputValue(const int index, const bool value);

protected:
    /**
     * \brief Copies each predecessor's output into m_inputValues.
     * \return \c true if all inputs were successfully read.
     */
    bool updateInputs();

    /// Read-only view of the cached input values for use by subclass updateLogic().
    const QVector<bool> &inputs() const { return m_inputValues; }

private:
    Q_DISABLE_COPY(LogicElement)

    // --- Members ---

    QVector<InputPair> m_inputPairs;
    QVector<bool> m_inputValues;
    QVector<bool> m_outputValues;
    bool m_outputChanged = false; ///< Set by setOutputValue() when an output actually changes.
    bool m_isValid = true;
};

