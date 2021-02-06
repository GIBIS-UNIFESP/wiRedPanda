// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationcontroller.h"

#include "element/clock.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmapping.h"
#include "nodes/qneconnection.h"
#include "scene.h"
#include "simulationcontroller.h"

#include <QDebug>
#include <QGraphicsView>

SimulationController::SimulationController(Scene *scn)
    : QObject(dynamic_cast<QObject *>(scn))
    , elMapping(nullptr)
    , simulationTimer(this)
{
    scene = scn;
    simulationTimer.setInterval(GLOBALCLK);
    viewTimer.setInterval(int(1000 / 30));
    viewTimer.start();
    shouldRestart = false;
    connect(&viewTimer, &QTimer::timeout, this, &SimulationController::updateView);
    connect(&simulationTimer, &QTimer::timeout, this, &SimulationController::update);
}

SimulationController::~SimulationController()
{
    clear();
}

void SimulationController::updateScene(const QRectF &rect)
{
    if (canRun()) {
        const QList<QGraphicsItem *> &items = scene->items(rect);
        for (QGraphicsItem *item : items) {
            auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
            auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
            if (conn) {
                updateConnection(conn);
            } else if (elm && (elm->elementGroup() == ElementGroup::OUTPUT)) {
                auto const elm_inputs = elm->inputs();
                for (QNEInputPort *in : elm_inputs) {
                    updatePort(in);
                }
            }
        }
    }
}

void SimulationController::updateView()
{
    auto const scene_views = scene->views();
    updateScene(scene_views.first()->sceneRect());
}

void SimulationController::updateAll()
{
    updateScene(scene->itemsBoundingRect());
}

bool SimulationController::canRun()
{
    if (!elMapping) {
        return false;
    }
    return elMapping->canRun();
}

bool SimulationController::isRunning()
{
    return this->simulationTimer.isActive();
}

void SimulationController::update()
{
    if (shouldRestart) {
        shouldRestart = false;
        this->clear();
    }
    if (elMapping) {
        elMapping->update();
    }
}

void SimulationController::stop()
{
    simulationTimer.stop();
}

void SimulationController::start()
{
    qDebug() << "Start simulation controller.";
    Clock::reset = true;
    reSortElms();
    simulationTimer.start();
    qDebug() << "Simulation started.";
}

void SimulationController::reSortElms()
{
    qDebug() << "GENERATING SIMULATION LAYER";
    QVector<GraphicElement *> elements = scene->getElements();
    qDebug() << "Elements read:" << elements.size();
    if (elements.size() == 0) {
        return;
    }
    qDebug() << "After return.";
    if (elMapping) {
        delete elMapping;
    }
    qDebug() << "Elements deleted.";
    elMapping = new ElementMapping(scene->getElements(), GlobalProperties::currentFile);
    if (elMapping->canInitialize()) {
        qDebug() << "Can initialize.";
        elMapping->initialize();
        elMapping->sort();
        update();
    } else {
        qDebug() << "Cannot initialize simulation!";
        qDebug() << "Can not initialize.";
    }
    qDebug() << "Finished simulation layer.";
}

void SimulationController::clear()
{
    if (elMapping) {
        delete elMapping;
    }
    elMapping = nullptr;
}

void SimulationController::updatePort(QNEOutputPort *port)
{
    if (port) {
        GraphicElement *elm = port->graphicElement();
        Q_ASSERT(elm);
        LogicElement *logElm = nullptr;
        int portIndex = 0;
        if (elm->elementType() == ElementType::IC) {
            IC *ic = dynamic_cast<IC *>(elm);
            logElm = elMapping->getICMapping(ic)->getOutput(port->index());
        } else {
            logElm = elMapping->getLogicElement(elm);
            portIndex = port->index();
        }
        Q_ASSERT(logElm);
        if (logElm->isValid()) {
            port->setValue(logElm->getOutputValue(portIndex));
        } else {
            port->setValue(-1);
        }
    }
}

void SimulationController::updatePort(QNEInputPort *port)
{
    Q_ASSERT(port);
    GraphicElement *elm = port->graphicElement();
    Q_ASSERT(elm);
    LogicElement *logElm = elMapping->getLogicElement(elm);
    Q_ASSERT(logElm);
    int portIndex = port->index();
    if (logElm->isValid()) {
        port->setValue(logElm->getInputValue(portIndex));
    } else {
        port->setValue(-1);
    }
    if (elm->elementGroup() == ElementGroup::OUTPUT) {
        elm->refresh();
    }
}

void SimulationController::updateConnection(QNEConnection *conn)
{
    Q_ASSERT(conn);
    updatePort(conn->start());
}
