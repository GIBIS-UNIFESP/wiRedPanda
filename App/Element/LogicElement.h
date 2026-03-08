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

    // --- Validity & Feedback ---

    /// Returns \c true if all required input ports are connected.
    bool isValid() const;
    /// Returns \c true if this element is part of a combinational feedback loop.
    bool inFeedbackLoop() const;
    /// Recomputes and caches the validity state.
    void validate();

    // --- I/O Values ---

    /// Returns the current logic value on input port \a index.
    bool inputValue(const int index = 0) const;
    /// Returns the current logic value on output port \a index.
    bool outputValue(const int index = 0) const;
    /// Returns the number of output ports.
    int outputSize() const;

    // --- Priority & Logic ---

    /// Computes and returns the topological depth of this element.
    int calculatePriority();
    /// Returns the cached topological priority (higher = closer to inputs, updated first).
    int priority() const;
    virtual void updateLogic() = 0;

    // --- Connection management ---

    /** \brief Connects input \a index of this element to output \a port of \a logic. */
    void connectPredecessor(const int index, LogicElement *logic, const int port);
    /// Removes all successor references from this element.
    void clearSucessors();

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

    QVector<bool> m_inputValues; ///< Cached input values refreshed each simulation step.

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

