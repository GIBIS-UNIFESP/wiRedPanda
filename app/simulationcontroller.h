/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "elementmapping.h"

#include <QObject>
#include <QTimer>
#include <memory>

class QNEInputPort;
class QNEOutputPort;
class Scene;

class SimulationController : public QObject
{
    Q_OBJECT

public:
    explicit SimulationController(Scene *scene);
    ~SimulationController() override = default;

    bool initialize();
    bool isRunning();
    void restart();
    void start();
    void stop();
    void update();
    void updateScene();

private:
    static void updatePort(QNEInputPort *port);

    bool canRun() const;
    void updatePort(QNEOutputPort *port);

    QTimer m_simulationTimer;
    QTimer m_viewTimer;
    Scene *m_scene;
    bool m_initialized = false;
    std::unique_ptr<ElementMapping> m_elmMapping;
};
