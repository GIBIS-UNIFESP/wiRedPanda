/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

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
    // If m_shouldRestart == true, then the simulation controller will be cleared the next time that
    // it is updated.
    void setRestart() { m_shouldRestart = true; }

    explicit SimulationController(Scene *scn);
    ~SimulationController() override;

    void updateScene(const QRectF &rect);
    static QVector<GraphicElement *> sortElements(QVector<GraphicElement *> elms);

    bool isRunning();
    void clear();
    void startTimer();

signals:

public slots:
    void update();
    void stop();
    void start();
    void tic();
    void updateView();
    void updateAll();
    bool canRun();
    void reSortElements();    
private:
    void updatePort(QNEOutputPort *port);
    void updatePort(QNEInputPort *port);
    void updateConnection(QNEConnection *conn);

    bool m_shouldRestart;
    ElementMapping *m_elMapping;
    Scene *m_scene;
    QTimer m_simulationTimer;
    QTimer m_viewTimer;
};

#endif /* SIMULATIONCONTROLLER_H */
