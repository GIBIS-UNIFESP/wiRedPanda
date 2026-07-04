// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Per-element simulation runtime state (input/output values and the connection graph).
 */

#pragma once

#include <QVector>

#include "App/Core/Enums.h"

class GraphicElement;
class InputPort;
class OutputPort;

/**
 * \class ElementSimState
 * \brief Owns a GraphicElement's simulation runtime state, decoupled from its graphics.
 *
 * \details Holds the cached input/output four-state values, the output-changed flag, and
 * the simulation-graph edges feeding each input.  The propagation logic reads the owning
 * element's ports only through parameters (no back-pointer), so this type carries no Qt
 * graphics dependency.  GraphicElement holds one as a value member and forwards its
 * direct-simulation interface to it.
 */
class ElementSimState
{
public:
    /// A single simulation-graph edge: which element/output port feeds one input slot.
    struct InputConnection {
        GraphicElement *sourceElement = nullptr;
        int sourceOutputIndex = 0;
    };

    /// Allocates the I/O vectors and seeds outputs from \a outputPorts default statuses.
    void initVectors(int inputCount, int outputCount, const QVector<OutputPort *> &outputPorts);

    /// Resets each output slot to its port's power-on default (Unknown coerced to Inactive).
    void reset(const QVector<OutputPort *> &outputPorts);

    /// Records that simulation input \a inputIndex is driven by \a source output \a outputPort.
    void connectPredecessor(int inputIndex, GraphicElement *source, int outputPort);

    /**
     * \brief Snapshots each predecessor's output into the input cache, reading \a inputPorts
     * for unconnected-input defaults and multi-driver conflict detection.
     * \param allowUnknown When true, only a truly unconnected Unknown input fails (combinational
     * domination rules can still short-circuit); when false any Unknown/Error fails.
     * \return true if simulation can proceed; false (all outputs set Unknown) otherwise.
     */
    bool updateInputs(bool allowUnknown, const QVector<InputPort *> &inputPorts);

    /// Decodes \a count select-line statuses starting at \a offset into a binary index.
    int decodeSelectValue(int offset, int count) const;

    /// Returns the four-state value on output slot \a index (Unknown if out of range).
    Status outputValue(const int index = 0) const
    {
        if (index >= m_outputs.size()) { return Status::Unknown; }
        return m_outputs.at(index);
    }

    /// Sets output slot \a index to \a value, flagging a change when it differs.
    void setOutputValue(const int index, const Status value)
    {
        if (index >= m_outputs.size()) { return; }
        if (m_deferCommit) {
            // Synchronous sequential element mid-tick: stage the value so peers
            // still read the old output. commitDeferredOutputs() publishes it.
            m_staged[index] = value;
            return;
        }
        if (m_outputs[index] != value) { m_outputChanged = true; }
        m_outputs[index] = value;
    }

    /// Returns the number of simulation output slots.
    qsizetype outputSize() const { return m_outputs.size(); }

    /// Returns true if any output changed since the flag was last cleared.
    bool outputChanged() const { return m_outputChanged; }

    /// Clears the output-changed flag.
    void clearOutputChanged() { m_outputChanged = false; }

    /// Read-only view of the cached simulation input values.
    const QVector<Status> &inputs() const { return m_inputs; }

    /// Read-only view of the current simulation output values.
    /// During a deferred-commit window returns the staged (in-progress) outputs
    /// so a sequential element re-evaluated multiple times within one tick sees
    /// the value it already staged rather than its stale committed output.
    const QVector<Status> &outputs() const { return m_deferCommit ? m_staged : m_outputs; }

    /// Begins a deferred (non-blocking) commit window: seeds the staging buffer
    /// from current outputs and routes subsequent setOutputValue() calls to it.
    void beginDeferredCommit()
    {
        m_staged = m_outputs;
        m_deferCommit = true;
    }

    /// Ends the deferred-commit window and publishes staged outputs via the
    /// normal change-detecting path so visuals refresh correctly.
    void commitDeferredOutputs()
    {
        m_deferCommit = false;
        for (int i = 0; i < m_staged.size() && i < m_outputs.size(); ++i) {
            setOutputValue(i, m_staged.at(i));
        }
    }

private:
    QVector<InputConnection> m_connections;
    QVector<Status> m_inputs;
    QVector<Status> m_outputs;
    /// Staging buffer for non-blocking (synchronous) sequential commit. While
    /// m_deferCommit is true, setOutputValue() writes here so peers keep reading
    /// the pre-tick value until commitDeferredOutputs() publishes the snapshot.
    QVector<Status> m_staged;
    bool m_outputChanged = false;
    bool m_deferCommit = false;
};
