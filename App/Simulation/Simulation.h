// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Synchronous cycle-based simulation engine with event-driven clock support.
 */

#pragma once

#include <QGraphicsItem>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QTimer>

class Clock;
class GraphicElement;
class GraphicElementInput;
class QNEConnection;
class QNEInputPort;
class QNEOutputPort;
class Scene;

/**
 * \class Simulation
 * \brief Manages the digital circuit simulation loop.
 */
class Simulation : public QObject
{
    Q_OBJECT

public:
    explicit Simulation(Scene *scene);

    ~Simulation() override = default;

    // --- Control ---

    void start();
    void stop();
    void restart();
    bool isRunning();
    bool isInFeedbackLoop(const GraphicElement *element) const;

    // --- Initialization ---

    bool initialize();

    // --- Step ---

    void update();

    // --- Static graph building (used by IC::initializeSimulation too) ---
    static void buildConnectionGraph(const QVector<GraphicElement *> &elements);

signals:
    /// Emitted (at most once per initialize()) when a feedback circuit fails to converge.
    void simulationWarning(const QString &message);

private:
    Q_DISABLE_COPY(Simulation)

    static void updatePort(QNEInputPort *port);
    static void updatePort(QNEOutputPort *port);
    void updateWithIterativeSettling();
    void sortSimElements(const QVector<GraphicElement *> &elements);

    // --- Members: Timer & element lists ---

    QTimer m_timer;
    QVector<Clock *> m_clocks;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;
    QVector<QNEConnection *> m_connections;

    Scene *m_scene;

    bool m_initialized = false;
    bool m_convergenceWarned = false;

    QVector<GraphicElement *> m_sortedElements;
    QHash<const GraphicElement *, int> m_simPriorities;
    QSet<const GraphicElement *> m_simFeedbackNodes;
    bool m_simHasFeedbackElements = false;
};

