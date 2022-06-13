// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationcontroller.h"

#include "buzzer.h"
#include "clock.h"
#include "common.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmapping.h"
#include "qneconnection.h"
#include "scene.h"

#include <QGraphicsView>

SimulationController::SimulationController(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    m_simulationTimer.setInterval(GlobalProperties::globalClock);
    connect(&m_simulationTimer, &QTimer::timeout, this, &SimulationController::update);

    m_viewTimer.setInterval(1000 / 30);
    m_viewTimer.start();
    connect(&m_viewTimer,       &QTimer::timeout, this, &SimulationController::updateScene);
}

SimulationController::~SimulationController()
{
    clear();
}

void SimulationController::restart() { m_initialized = false; }

void SimulationController::updateScene()
{
    if (!canRun()) {
        return;
    }

    const auto items = m_scene->items();

    for (auto *item : items) {
        if (auto *connection = qgraphicsitem_cast<QNEConnection *>(item)) {
            updatePort(connection->start());
            continue;
        }

        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item); element && element->elementGroup() == ElementGroup::Output) {
            const auto inputs = element->inputs();
            for (auto *input : inputs) {
                updatePort(input);
            }
        }
    }
}

bool SimulationController::canRun()
{
    return m_initialized;
}

bool SimulationController::isRunning()
{
    return m_simulationTimer.isActive();
}

void SimulationController::update()
{
    if (!m_initialized) {
        initialize();
    }

    if (!m_elmMapping) { // TODO: Remove this check, if possible. May increse the simulation speed significantly.
        return;
    }

    m_elmMapping->update();
}

void SimulationController::stop()
{
    m_simulationTimer.stop();
    m_viewTimer.stop();
    m_scene->mute(true);
}

void SimulationController::start()
{
    qCDebug(zero) << tr("Starting simulation controller.");
    if (!m_initialized) {
        Clock::reset = true;
        initialize();
    }
    m_simulationTimer.start();
    m_viewTimer.start();
    qCDebug(zero) << tr("Simulation started.");
    m_scene->mute(false);
}

void SimulationController::initialize()
{
    qCDebug(two) << tr("GENERATING SIMULATION LAYER.");
    auto elements = m_scene->elements();
    qCDebug(zero) << tr("Elements read:") << elements.size();
    if (elements.empty()) {
        return;
    }
    qCDebug(two) << tr("Deleting existing mapping.");
    delete m_elmMapping;
    qCDebug(two) << tr("Recreating mapping for simulation.");
    m_elmMapping = new ElementMapping(elements);
    if (!m_elmMapping->canInitialize()) {
        qCDebug(zero) << tr("Cannot initialize simulation.");
        return;
    }
    qCDebug(two) << tr("Can initialize.");
    m_elmMapping->initialize();
    qCDebug(two) << tr("Sorting.");
    m_elmMapping->sort();
    m_initialized = true;
    qCDebug(zero) << tr("Finished simulation layer.");
}

void SimulationController::clear()
{
    delete m_elmMapping;
    m_elmMapping = nullptr;
}

void SimulationController::updatePort(QNEOutputPort *port)
{
    if (!port) {
        return;
    }

    GraphicElement *elm = port->graphicElement();
    LogicElement *logElm = nullptr;
    int portIndex = 0;

    if (elm->elementType() == ElementType::IC) {
        IC *ic = dynamic_cast<IC *>(elm);
        logElm = m_elmMapping->icMapping(ic)->output(port->index());
    } else {
        logElm = elm->logic();;
        portIndex = port->index();
    }

    if (logElm->isValid()) {
        port->setValue(static_cast<Status>(logElm->outputValue(portIndex)));
    } else {
        port->setValue(Status::Invalid);
    }
}

void SimulationController::updatePort(QNEInputPort *port)
{
    GraphicElement *elm = port->graphicElement();
    LogicElement *logElm = elm->logic();
    int portIndex = port->index();

    if (logElm->isValid()) {
        port->setValue(static_cast<Status>(logElm->inputValue(portIndex)));
    } else {
        port->setValue(Status::Invalid);
    }

    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}
