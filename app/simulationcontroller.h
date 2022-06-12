/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QTimer>

class Clock;
class ElementMapping;
class GraphicElement;
class QNEConnection;
class QNEInputPort;
class QNEOutputPort;
class Scene;

class SimulationController : public QObject
{
    Q_OBJECT

public:
    explicit SimulationController(Scene *scene);
    ~SimulationController() override;

    bool isRunning();
    void clear();
    void initialize();
    void restart();
    void start();
    void stop();
    void update();
    void updateScene();

private:
    static void updatePort(QNEInputPort *port);

    bool canRun();
    void updatePort(QNEOutputPort *port);

    ElementMapping *m_elmMapping = nullptr;
    QTimer m_simulationTimer;
    QTimer m_viewTimer;
    Scene *m_scene;
    bool m_shouldRestart = false;
};
