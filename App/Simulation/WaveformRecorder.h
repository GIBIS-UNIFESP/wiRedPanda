// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Per-signal transition recording for the temporal simulation waveform viewer.
 */

#pragma once

#include <algorithm>

#include <QPair>
#include <QPointer>
#include <QString>
#include <QVector>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Simulation/SimTime.h"

/**
 * \struct SignalTrace
 * \brief Records timestamped transitions for a single signal.
 */
struct SignalTrace
{
    QString name;                                  ///< Display name (e.g., "AND_0 [out 0]").
    QPointer<GraphicElement> logic;                ///< Element being watched (auto-nulls when freed).
    int portIndex = 0;                             ///< Output port index on the element.
    QVector<QPair<SimTime, Status>> transitions;   ///< Chronological (time, value) pairs.

    /// Returns the status at a given simulation time (binary search).
    Status statusAt(SimTime time) const
    {
        if (transitions.isEmpty()) {
            return Status::Inactive;
        }
        // Find the last transition at or before `time`.
        qsizetype lo = 0, hi = transitions.size() - 1;
        if (time < transitions.first().first) {
            return Status::Inactive;
        }
        while (lo < hi) {
            qsizetype mid = lo + (hi - lo + 1) / 2;
            if (transitions[mid].first <= time) {
                lo = mid;
            } else {
                hi = mid - 1;
            }
        }
        return transitions[lo].second;
    }
};

/**
 * \class WaveformRecorder
 * \brief Manages a set of watched signals and their recorded transitions.
 *
 * \details Call watchSignal() to add signals, then recordAll() on each temporal
 * simulation tick to capture transitions.  The recorder is owned by the
 * Simulation; traces hold a QPointer to the watched element, so a freed element
 * auto-nulls (and is reaped by pruneStale() on the next rebuild) rather than dangling.
 */
class WaveformRecorder
{
public:
    WaveformRecorder() = default;

    // --- Signal management ---

    /// Adds a signal to the watch list.
    /// \return Index of the new trace.
    int watchSignal(const QString &name, GraphicElement *logic, int portIndex)
    {
        const int idx = static_cast<int>(m_traces.size());
        m_traces.append({name, logic, portIndex, {}});
        return idx;
    }

    /// Removes all watched signals and recorded data.
    void clear() { m_traces.clear(); }

    /// Clears every trace's recorded transition history while preserving the watch list
    /// itself (name/logic/portIndex survive). Call whenever the owning Simulation's time-base
    /// restarts at 0 — the old timeline and the new one are never comparable, so continuing to
    /// append to the same trace would violate the ascending-time order statusAt() and the
    /// waveform viewer both assume, producing wrong lookups and spurious glitch lines. There is
    /// no separate "last recorded value" field to also reset: recordAll()'s dedup reads
    /// transitions.last() directly, so clearing transitions resets that state for free.
    void resetTimeline()
    {
        for (auto &tr : m_traces) {
            tr.transitions.clear();
        }
    }

    /// Returns true if every trace's transition history is currently empty. Used to assert the
    /// invariant that resetTimeline() accompanies every simulation time-base reset.
    bool timelineEmpty() const
    {
        return std::all_of(m_traces.cbegin(), m_traces.cend(),
                            [](const SignalTrace &tr) { return tr.transitions.isEmpty(); });
    }

    /// Drops traces whose watched element has been destroyed (its QPointer went null). Called on
    /// simulation rebuild (Simulation::initialize()) so the recorder participates in the
    /// netlist-invalidation invariant: dead watches are reaped, while live ones — whose element
    /// survives the rebuild — keep recording. This is what makes the QPointer in SignalTrace::logic
    /// not just crash-safe but self-healing, without ever clobbering a still-valid signal.
    void pruneStale()
    {
        m_traces.removeIf([](const SignalTrace &tr) { return tr.logic.isNull(); });
    }

    /// Returns the number of watched signals.
    int traceCount() const { return static_cast<int>(m_traces.size()); }

    /// Returns the trace at \a index.
    const SignalTrace &trace(int index) const { return m_traces[index]; }

    /// Returns all traces.
    const QVector<SignalTrace> &traces() const { return m_traces; }

    // --- Recording ---

    /// Snapshots the current output value of all watched signals at \a time.
    /// Only records if the value differs from the last recorded value (dedup).
    void recordAll(SimTime time)
    {
        for (auto &tr : m_traces) {
            // Resolve the guarded pointer once: QPointer::operator-> re-checks the target on
            // every access, so a separate `if (!tr.logic)` guard followed by `tr.logic->...`
            // reads as a possible null dereference under optimization (-Werror=null-dereference).
            // A single raw pointer after the guard is provably non-null for the rest of the loop.
            GraphicElement *logic = tr.logic;
            if (!logic) {
                continue;
            }
            const Status current = logic->outputValue(tr.portIndex);
            if (tr.transitions.isEmpty()) {
                tr.transitions.append({time, current});
            } else if (tr.transitions.last().second != current) {
                // A zero-delay element firing exactly on a tick boundary can re-record the
                // same timestamp on the next tick with a new value; collapse that into a
                // single transition instead of emitting a zero-width glitch.
                if (tr.transitions.last().first == time) {
                    tr.transitions.last().second = current;
                } else {
                    tr.transitions.append({time, current});
                }
            }
        }
    }

    // --- Query ---

    /// Returns the latest recorded time across all traces (0 if empty).
    SimTime maxTime() const
    {
        SimTime max = 0;
        for (const auto &tr : m_traces) {
            if (!tr.transitions.isEmpty()) {
                max = qMax(max, tr.transitions.last().first);
            }
        }
        return max;
    }

    /// Returns true if recording is active.
    bool isRecording() const { return m_recording; }

    /// Enables or disables recording.
    void setRecording(bool on) { m_recording = on; }

private:
    QVector<SignalTrace> m_traces;
    bool m_recording = false;
};
