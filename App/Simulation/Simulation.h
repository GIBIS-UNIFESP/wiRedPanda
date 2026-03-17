// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Dual-mode simulation engine: functional (zero-delay) and temporal (event-driven).
 */

#pragma once

#include <memory>

#include <QGraphicsItem>
#include <QObject>
#include <QTimer>

#include "App/Simulation/ElementMapping.h"
#include "App/Simulation/SimEvent.h"
#include "App/Simulation/WaveformRecorder.h"

class LogicElement;
class QNEConnection;
class QNEInputPort;
class QNEOutputPort;
class Scene;

/// Selects the simulation evaluation strategy.
enum class SimulationMode {
    Functional, ///< Zero-delay, topological-sort pass (default — current behaviour).
    Temporal,   ///< Event-driven with per-element propagation delays.
};

/**
 * \class Simulation
 * \brief Manages the digital circuit simulation loop.
 *
 * \details The simulation runs a 1 ms periodic QTimer.  In **Functional** mode
 * each tick evaluates all logic in topological order (zero-delay).  In
 * **Temporal** mode each tick advances simulation time and processes events
 * from a priority queue, respecting per-element propagation delays.
 *
 * Feedback loops are detected during initialization.  In Functional mode they
 * use iterative settling; in Temporal mode delta-cycle capping handles them.
 */
class Simulation : public QObject
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /**
     * \brief Constructs a Simulation bound to \a scene.
     * \param scene Scene whose elements will be simulated.
     */
    explicit Simulation(Scene *scene);

    /// Destructor; stops the simulation timer.
    ~Simulation() override = default;

    // --- Control ---

    /// Starts the 1 ms simulation timer.
    void start();

    /// Stops the simulation timer.
    void stop();

    /// Stops and immediately restarts the simulation.
    void restart();

    /// Returns \c true if the simulation timer is currently running.
    bool isRunning();

    /// Returns \c true if \a logic is part of a combinational feedback loop.
    bool isInFeedbackLoop(const LogicElement *logic) const;

    // --- Mode ---

    /// Returns the current simulation mode.
    SimulationMode mode() const { return m_mode; }
    /// Sets the simulation mode.  Takes effect on the next restart / initialize().
    void setMode(SimulationMode mode);

    // --- Temporal mode ---

    /// Returns the current simulation time in nanoseconds (temporal mode only).
    SimTime currentTime() const { return m_currentTime; }

    /// Sets how many nanoseconds of simulation time advance per 1 ms wall-clock tick.
    /// At 1x speed this is 1 000 000 (1 ms).
    void setTimePerTick(SimTime ns) { m_timePerTick = ns; }

    /// Returns the current time-per-tick setting.
    SimTime timePerTick() const { return m_timePerTick; }

    // --- Waveform recording ---

    /// Returns the waveform recorder (for adding watches and reading traces).
    WaveformRecorder &waveformRecorder() { return m_recorder; }
    const WaveformRecorder &waveformRecorder() const { return m_recorder; }

    // --- Initialization ---

    /**
     * \brief Builds the ElementMapping from the current scene elements.
     * \return \c true if initialization succeeded (all elements are valid).
     */
    bool initialize();

    // --- Step ---

    /// Executes one simulation step (used by tests to advance the simulation manually).
    void update();

signals:
    /// Emitted (at most once per initialize()) when a feedback circuit fails to converge.
    void simulationWarning(const QString &message);

private:
    Q_DISABLE_COPY(Simulation)

    // --- Mode-specific update loops ---

    void updateFunctional();
    void updateTemporal();

    // --- Helpers ---

    static void updatePort(QNEInputPort *port);
    static void updatePort(QNEOutputPort *port);
    void updateWithIterativeSettling();
    void refreshVisuals();
    void scheduleSuccessors(LogicElement *source);

    // --- Members: Timer & element lists ---

    QTimer m_timer;
    QVector<Clock *> m_clocks;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<QNEConnection *> m_connections;

    // --- Members: Scene & mapping ---

    Scene *m_scene;
    std::unique_ptr<ElementMapping> m_elmMapping;

    // --- Members: Temporal engine ---

    SimulationMode m_mode = SimulationMode::Functional;
    EventQueue m_eventQueue;
    SimTime m_currentTime = 0;
    SimTime m_timePerTick = 1'000'000; ///< 1 ms in nanoseconds (1x speed).
    WaveformRecorder m_recorder;

    // --- Members: State flags ---

    bool m_hasFeedbackElements = false;
    bool m_initialized = false;
    bool m_convergenceWarned = false;
};
