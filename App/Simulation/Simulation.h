// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Synchronous cycle-based simulation engine with event-driven clock support.
 */

#pragma once

#include <memory>

#include <QGraphicsItem>
#include <QObject>
#include <QTimer>

#include "App/Simulation/ElementMapping.h"

class LogicElement;
class QNEConnection;
class QNEInputPort;
class QNEOutputPort;
class Scene;

/**
 * \class Simulation
 * \brief Manages the digital circuit simulation loop.
 *
 * \details The simulation runs a 1 ms periodic QTimer.  On each tick it:
 * 1. Updates all GraphicElementInput outputs (including clocks).
 * 2. Calls updateLogic() on all LogicElements in priority order.
 * 3. Propagates values through connections to output elements.
 *
 * Feedback loops are detected during initialization and handled with an
 * iterative settling algorithm.
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

    // --- Initialization ---

    /**
     * \brief Builds the ElementMapping from the current scene elements.
     * \return \c true if initialization succeeded (all elements are valid).
     */
    bool initialize();

    // --- Step ---

    /// Executes one simulation step (used by tests to advance the simulation manually).
    void update();

private:
    Q_DISABLE_COPY(Simulation)

    // --- Helpers ---

    static void updatePort(QNEInputPort *port);
    static void updatePort(QNEOutputPort *port);
    void updateWithIterativeSettling();

    // --- Members: Timer & element lists ---

    QTimer m_timer;
    QVector<Clock *> m_clocks;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<QNEConnection *> m_connections;
    QVector<QVector<Status>> m_previousOutputs;

    // --- Members: Scene & mapping ---

    Scene *m_scene;
    std::unique_ptr<ElementMapping> m_elmMapping;

    // --- Members: State flags ---

    bool m_hasFeedbackElements = false;
    bool m_initialized = false;
};
