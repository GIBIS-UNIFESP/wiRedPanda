// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationcontroller.h"

#include "clock.h"
#include "common.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmapping.h"
#include "qneconnection.h"
#include "scene.h"
#include "simulationcontroller.h"

#include <QDebug>
#include <QGraphicsView>

SimulationController::SimulationController(Scene *scn)
    : QObject(scn)
    , m_shouldRestart(false)
    , m_elMapping(nullptr)
    , m_scene(scn)
    , m_simulationTimer(this)
{
    m_simulationTimer.setInterval(globalClock);
    m_viewTimer.setInterval(int(1000 / 30));
    m_viewTimer.start();
    connect(&m_viewTimer, &QTimer::timeout, this, &SimulationController::updateView);
    connect(&m_simulationTimer, &QTimer::timeout, this, &SimulationController::tic);
}

SimulationController::~SimulationController()
{
    clear();
}

void SimulationController::updateScene(const QRectF &rect)
{
    if (!canRun()) {
        return;
    }

    for (auto *item : m_scene->items(rect)) {
        auto *connection = qgraphicsitem_cast<QNEConnection *>(item);
        auto *element = qgraphicsitem_cast<GraphicElement *>(item);

        if (connection) {
            updateConnection(connection);
            continue;
        }

        if (element && element->elementGroup() == ElementGroup::Output) {
            for (auto *input : element->inputs()) {
                updatePort(input);
            }
        }
    }
}

void SimulationController::updateView()
{
    const auto scene_views = m_scene->views();
    updateScene(scene_views.first()->sceneRect());
}

void SimulationController::updateAll()
{
    updateScene(m_scene->itemsBoundingRect());
}

bool SimulationController::canRun()
{
    return m_elMapping ? m_elMapping->canRun() : false;
}

bool SimulationController::isRunning()
{
    return m_simulationTimer.isActive();
}

void SimulationController::tic()
{
    update();
}

void SimulationController::update()
{
    if (m_shouldRestart) {
        m_shouldRestart = false;
        clear();
    }
    if (m_elMapping) { // TODO: Remove this check, if possible. May increse the simulation speed significantly.
        m_elMapping->update();
    }
}

void SimulationController::stop()
{
    m_simulationTimer.stop();
}

void SimulationController::startTimer()
{
    COMMENT("Starting timer.", 0);
    m_simulationTimer.start();
}

void SimulationController::start()
{
    COMMENT("Start simulation controller.", 0);
    Clock::reset = true;
    reSortElements();
    m_simulationTimer.start();
    COMMENT("Simulation started.", 0);
}

void SimulationController::reSortElements()
{
    COMMENT("GENERATING SIMULATION LAYER", 2);
    QVector<GraphicElement *> elements = m_scene->getElements();
    COMMENT("Elements read: " << elements.size(), 0);
    if (elements.empty()) {
        return;
    }
    COMMENT("Deleting existing mapping.", 2);
    delete m_elMapping;
    COMMENT("Recreating mapping for simulation.", 2);
    m_elMapping = new ElementMapping(elements);
    if (m_elMapping->canInitialize()) {
        COMMENT("Can initialize.", 2);
        m_elMapping->initialize();
        COMMENT("Sorting.", 2);
        m_elMapping->sort();
        COMMENT("Updating.", 2);
        update();
    } else {
        qDebug() << "Cannot initialize simulation!";
        COMMENT("Can not initialize.", 0);
    }
    COMMENT("Finished simulation layer.", 0);
}

void SimulationController::clear()
{
    delete m_elMapping;
    m_elMapping = nullptr;
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
            logElm = m_elMapping->getICMapping(ic)->getOutput(port->index());
        } else {
            logElm = m_elMapping->getLogicElement(elm);
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
    LogicElement *logElm = m_elMapping->getLogicElement(elm);
    Q_ASSERT(logElm);
    int portIndex = port->index();
    if (logElm->isValid()) {
        port->setValue(logElm->getInputValue(portIndex));
    } else {
        port->setValue(-1);
    }
    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}

void SimulationController::updateConnection(QNEConnection *conn)
{
    Q_ASSERT(conn);
    updatePort(conn->start());
}
