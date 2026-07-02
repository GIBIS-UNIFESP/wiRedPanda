// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief RAII guard that temporarily disables waveform recording while in scope.
 */

#pragma once

#include <QObject>

class Simulation;

/**
 * \class SimulationRecordingSuspender
 * \brief RAII guard that disables the simulation's waveform recorder on construction and
 * restores its prior recording state on destruction.
 *
 * \details A synthetic sweep (e.g. BeWavedDolphin's column-by-column re-evaluation) drives the
 * same Simulation the Live Analyzer may be watching. Without this guard, a sweep's synthetic
 * transitions would be written into the same traces the analyzer displays — polluting a live
 * circuit trace with unrelated test-vector data. Guarding recording (not just clearing the
 * timeline afterward) means the sweep's data is never recorded in the first place.
 *
 * Example:
 * \code
 * {
 *     SimulationRecordingSuspender recordingSuspender(simulation);
 *     // ... drive a synthetic sweep over the same Simulation ...
 * } // recording state (on/off) is restored here, whatever it was before
 * \endcode
 */
class SimulationRecordingSuspender
{
public:
    /**
     * \brief Saves \a simulation's current recording state and disables recording.
     * \param simulation Simulation whose recorder is being guarded.
     */
    explicit SimulationRecordingSuspender(Simulation *simulation);

    /// Restores the recording state saved at construction.
    ~SimulationRecordingSuspender();

private:
    Q_DISABLE_COPY(SimulationRecordingSuspender)

    Simulation *m_simulation;     ///< The guarded simulation instance.
    bool m_wasRecording = false;  ///< Recording state to restore on destruction.
};
