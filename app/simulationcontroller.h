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

    static QVector<GraphicElement *> sortElements(QVector<GraphicElement *> elms);

    bool isRunning();
    void clear();
    void reSortElements();
    void setRestart();
    void start();
    void startTimer();
    void stop();
    void update();
    void updateAll();
    void updateScene(const QRectF &rect);

private:
    bool canRun();
    void tic();
    void updatePort(QNEInputPort *port);
    void updatePort(QNEOutputPort *port);
    void updateView();

    ElementMapping *m_elmMapping = nullptr;
    QTimer m_simulationTimer;
    QTimer m_viewTimer;
    Scene *m_scene;
    bool m_shouldRestart = false;
};

