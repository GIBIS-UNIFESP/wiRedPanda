// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Base class for all simulation-layer logic elements.
 */

#pragma once

#include <QVector>

#include "App/Core/Enums.h"
#include "App/Simulation/SimTime.h"

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
 * \struct OutputPair
 * \brief Links an output port of one LogicElement to an input slot of another.
 */
struct OutputPair
{
    LogicElement *logic = nullptr; ///< Successor logic element.
    int inputPort = 0;             ///< Input port index on the successor.
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

    // --- I/O Values ---

    /// Returns the tri-state signal value on output port \a index.
    Status outputValue(const int index = 0) const;
    /// Returns the number of output ports.
    int outputSize() const;

    // --- Propagation delay ---

    /// Returns the propagation delay in nanoseconds (0 = instant, used in functional mode).
    SimTime propagationDelay() const { return m_propagationDelay; }
    /// Sets the propagation delay in nanoseconds.
    void setPropagationDelay(const SimTime ns) { m_propagationDelay = ns; }

    // --- Successor tracking ---

    /// Returns the successor list for output port \a outputIndex.
    const QVector<OutputPair> &successors(const int outputIndex) const { return m_successors[outputIndex]; }
    /// Returns the number of output-port successor lists.
    int successorGroupCount() const { return m_successors.size(); }

    /// Initializes successor storage for \a count output ports.  Called by ElementMapping.
    void initSuccessors(const int count) { m_successors.resize(count); }
    /// Appends a successor to output port \a outputIndex.  Called by ElementMapping.
    void addSuccessor(const int outputIndex, const OutputPair &pair) { m_successors[outputIndex].append(pair); }

    // --- Change tracking ---

    /// Returns \c true if any output changed during the last updateLogic() call.
    bool outputChanged() const { return m_outputChanged; }
    /// Clears the output-changed flag (call before updateLogic() to prepare for change detection).
    void clearOutputChanged() { m_outputChanged = false; }

    // --- Non-blocking assignment (two-phase delta cycles) ---

    /// Saves current outputs and clears the changed flag before evaluation.
    void prepareForEvaluation();
    /// Captures computed outputs into the pending buffer and restores pre-evaluation values.
    void deferOutputs();
    /// Applies pending outputs to live values.  Returns \c true if any output actually changed.
    bool commitOutputs();

    // --- Logic ---

    const QVector<InputPair> &inputPairs() const;
    virtual void updateLogic() = 0;

    // --- Connection management ---

    /** \brief Connects input \a index of this element to output \a port of \a logic. */
    void connectPredecessor(const int index, LogicElement *logic, const int port);

    // --- Output setting ---

    /// Sets all output ports to \a value.
    void setOutputValue(const Status value);
    /// Sets output port \a index to \a value.
    void setOutputValue(const int index, const Status value);

    /// Convenience overload — converts \c bool to Active/Inactive.
    void setOutputValue(const bool value) { setOutputValue(value ? Status::Active : Status::Inactive); }
    /// Convenience overload — converts \c bool to Active/Inactive.
    void setOutputValue(const int index, const bool value) { setOutputValue(index, value ? Status::Active : Status::Inactive); }

protected:
    /**
     * \brief Snapshots each predecessor's output into the input cache.
     *
     * If any predecessor output is Invalid (unconnected port), all outputs are
     * set to Invalid and the method returns \c false so that \c updateLogic()
     * implementations can perform an early return without computing a result
     * from incomplete data.
     *
     * \return \c true if all inputs are Active or Inactive (simulation can proceed).
     */
    bool updateInputs();

    /// Read-only view of the cached input values for use by subclass updateLogic().
    const QVector<Status> &inputs()  const { return m_inputValues; }
    /// Read-only view of the current output values for use by subclass updateLogic().
    const QVector<Status> &outputs() const { return m_outputValues; }

private:
    Q_DISABLE_COPY(LogicElement)

    /// Traverses the predecessor pointer to read the live value on input port \a index.
    Status inputValue(const int index = 0) const;

    // --- Members ---

    QVector<InputPair> m_inputPairs;
    QVector<Status> m_inputValues;  ///< Cached input values refreshed each simulation step.
    QVector<Status> m_outputValues;
    QVector<QVector<OutputPair>> m_successors; ///< [outputIndex] → list of successor (element, inputPort) pairs.
    QVector<Status> m_pendingOutputs; ///< Deferred output buffer for non-blocking assignment.
    QVector<Status> m_savedOutputs;   ///< Snapshot for restore during deferOutputs().
    SimTime m_propagationDelay = 0;   ///< Propagation delay in nanoseconds (0 = instant).
    bool m_outputChanged = false;     ///< Set by setOutputValue() when an output actually changes.
};
