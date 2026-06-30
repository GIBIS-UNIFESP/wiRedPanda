// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Per-element simulation runtime state (input/output values and the connection graph).
 */

#pragma once

#include <algorithm>

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

    /// Number of simulation input slots (predecessor links).
    int connectionCount() const { return static_cast<int>(m_connections.size()); }

    /// Element feeding simulation input slot \a index, or \c nullptr if unconnected or out of range.
    GraphicElement *predecessor(int index) const
    {
        if (index >= m_connections.size()) { return nullptr; }
        return m_connections[index].sourceElement;
    }

    /**
     * \brief Snapshots each predecessor's output into the input cache, reading \a inputPorts
     * for unconnected-input defaults and multi-driver conflict detection.
     * \param inputPorts Input ports, read for unconnected-input defaults and multi-driver
     * conflict detection.
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
            if (m_staged[index] != value) { m_stagedChanged = true; }
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
        // IDEMPOTENT while a window is already open. The engine calls this before EVERY
        // evaluation, so re-seeding the staging buffer here would discard a result still
        // awaiting publication -- a flip-flop re-evaluated inside its own propagation delay
        // would lose the value it had just captured, and the guarantee outputs() documents
        // below ("sees the value it already staged") could not hold. Reopening is only correct
        // once the pending publish has committed, which is exactly when m_deferCommit is false
        // again.
        if (m_deferCommit) { return; }

        // Element-wise copy into persistent storage, deliberately NOT `m_staged =
        // m_outputs`: assignment CoW-shares the two buffers, so every staged write and
        // every commit write into m_outputs pays a detach (malloc + deep copy) -- per
        // sequential element, per simulation tick, even when nothing changed. The copy
        // keeps both buffers owned and the whole tick allocation-free. The resize is a
        // no-op in steady state (initVectors() sizes m_staged); it only guards a
        // mid-run port-count change.
        m_staged.resize(m_outputs.size());
        std::copy(m_outputs.cbegin(), m_outputs.cend(), m_staged.begin());
        m_deferCommit = true;
    }

    /// Returns \c true while a deferred-commit window is open (a staged result is awaiting
    /// publication). Lets the engine tell "first evaluation after a commit" from "another
    /// evaluation inside a delay that has not elapsed yet".
    bool isDeferCommitOpen() const { return m_deferCommit; }

    /// Returns \c true if a staged write changed a value since clearStagedChanged().
    bool stagedChanged() const { return m_stagedChanged; }

    /// Clears the staged-changed flag; call immediately before re-evaluating.
    void clearStagedChanged() { m_stagedChanged = false; }

    /// Closes a deferred-commit window and DISCARDS what it staged. For callers that write
    /// m_outputs directly and must not have their write routed into staging.
    void discardDeferredCommit() { m_deferCommit = false; m_stagedChanged = false; }

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
    bool m_stagedChanged = false;
};
