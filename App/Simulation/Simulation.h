// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QGraphicsItem>
#include <QObject>
#include <QTimer>

#include "App/Simulation/ElementMapping.h"

class QNEConnection;
class QNEInputPort;
class QNEOutputPort;
class Scene;

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

    // --- Initialization ---

    bool initialize();

    // --- Step ---

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

    // --- Members: Scene & mapping ---

    Scene *m_scene;
    std::unique_ptr<ElementMapping> m_elmMapping;

    // --- Members: State flags ---

    bool m_initialized = false;
};

