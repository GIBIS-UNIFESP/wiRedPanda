// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Per-signal transition recording for the temporal simulation waveform viewer.
 */

#pragma once

#include <QPair>
#include <QString>
#include <QVector>

#include "App/Core/Enums.h"
#include "App/Simulation/SimTime.h"

class LogicElement;

/**
 * \struct SignalTrace
 * \brief Records timestamped transitions for a single signal.
 */
struct SignalTrace
{
    QString name;                                  ///< Display name (e.g., "AND_0 [out 0]").
    LogicElement *logic = nullptr;                 ///< Element being watched.
    int portIndex = 0;                             ///< Output port index on the element.
    QVector<QPair<SimTime, Status>> transitions;   ///< Chronological (time, value) pairs.

    /// Returns the status at a given simulation time (binary search).
    Status statusAt(SimTime time) const
    {
        if (transitions.isEmpty()) {
            return Status::Inactive;
        }
        // Find the last transition at or before `time`.
        int lo = 0, hi = transitions.size() - 1;
        if (time < transitions.first().first) {
            return Status::Inactive;
        }
        while (lo < hi) {
            int mid = lo + (hi - lo + 1) / 2;
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
 * \details Call watchSignal() to add signals, then recordIfChanged() on each
 * temporal simulation event to capture transitions.  The recorder is owned by
 * the Simulation and reset on restart.
 */
class WaveformRecorder
{
public:
    WaveformRecorder() = default;

    // --- Signal management ---

    /// Adds a signal to the watch list.
    /// \return Index of the new trace.
    int watchSignal(const QString &name, LogicElement *logic, int portIndex)
    {
        int idx = m_traces.size();
        m_traces.append({name, logic, portIndex, {}});
        return idx;
    }

    /// Removes all watched signals and recorded data.
    void clear() { m_traces.clear(); }

    /// Returns the number of watched signals.
    int traceCount() const { return m_traces.size(); }

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
            if (!tr.logic) {
                continue;
            }
            const Status current = tr.logic->outputValue(tr.portIndex);
            if (tr.transitions.isEmpty() || tr.transitions.last().second != current) {
                tr.transitions.append({time, current});
            }
        }
    }

    /// Records a single signal if its value changed.
    void recordIfChanged(int traceIndex, SimTime time)
    {
        auto &tr = m_traces[traceIndex];
        if (!tr.logic) {
            return;
        }
        const Status current = tr.logic->outputValue(tr.portIndex);
        if (tr.transitions.isEmpty() || tr.transitions.last().second != current) {
            tr.transitions.append({time, current});
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
