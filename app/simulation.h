/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "elementmapping.h"

#include <QGraphicsItem>
#include <QObject>
#include <QTimer>
#include <memory>

class QNEInputPort;
class QNEOutputPort;
class Scene;

class Simulation : public QObject
{
    Q_OBJECT

public:
    explicit Simulation(Scene *scene);
    ~Simulation() override = default;

    bool initialize();
    bool isRunning();
    void restart();
    void start();
    void stop();
    void update();
    void updateScene();

private:
    Q_DISABLE_COPY(Simulation)

    static void updatePort(QNEInputPort *port);
    static void updatePort(QNEOutputPort *port);

    QList<QGraphicsItem *> m_items;
    QTimer m_timer;
    QVector<Clock *> m_clocks;
    QVector<GraphicElementInput *> m_inputs;
    Scene *m_scene;
    bool m_initialized = false;
    std::unique_ptr<ElementMapping> m_elmMapping;
};
