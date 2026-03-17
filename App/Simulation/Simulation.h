// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Event-driven simulation engine with blocking FIFO evaluation.
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
 * \details The simulation runs a 1 ms periodic QTimer.  On each tick it
 * generates events from clock toggles and input changes, then processes
 * them through a blocking FIFO queue until the circuit settles.
 * FIFO ordering breaks feedback symmetry without topological sorting.
 */
class Simulation : public QObject
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit Simulation(Scene *scene);
    ~Simulation() override = default;

    // --- Control ---

    void start();
    void stop();
    void restart();
    bool isRunning();

    /// Returns \c true if \a logic is part of a combinational feedback loop.
    bool isInFeedbackLoop(const LogicElement *logic) const;

    // --- Initialization ---

    bool initialize();

    // --- Step ---

    void update();

signals:
    void simulationWarning(const QString &message);

private:
    Q_DISABLE_COPY(Simulation)

    // --- Event-driven engine ---

    void processEvents();

    // --- Visual refresh ---

    static void updatePort(QNEInputPort *port);
    static void updatePort(QNEOutputPort *port);
    void refreshVisuals();

    // --- Members ---

    QTimer m_timer;
    QVector<Clock *> m_clocks;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<QNEConnection *> m_connections;

    Scene *m_scene;
    std::unique_ptr<ElementMapping> m_elmMapping;

    /// Elements whose outputs changed and whose successors need scheduling.
    QVector<LogicElement *> m_pendingSchedule;

    bool m_initialized = false;
    bool m_convergenceWarned = false;
};
