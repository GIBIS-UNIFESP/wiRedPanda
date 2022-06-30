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

using namespace std::chrono_literals;

SimulationController::SimulationController(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    m_simulationTimer.setInterval(1ms);
    connect(&m_simulationTimer, &QTimer::timeout, this, &SimulationController::update);

    m_viewTimer.setInterval(33ms);
    m_viewTimer.start();
    connect(&m_viewTimer,       &QTimer::timeout, this, &SimulationController::updateScene);
}

void SimulationController::update()
{
    if (!m_initialized && !initialize()) {
        return;
    }

    m_elmMapping->update();
}

void SimulationController::updateScene()
{
    if (!canRun()) {
        return;
    }

    const auto items = m_scene->items(Qt::SortOrder(-1));

    for (auto *item : items) {
        if (auto *connection = qgraphicsitem_cast<QNEConnection *>(item)) {
            updatePort(connection->start());
            continue;
        }

        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item);
                element && (element->elementGroup() == ElementGroup::Output)) {
            const auto ports = element->inputs();

            for (auto *port : ports) {
                updatePort(port);
            }
        }
    }
}

void SimulationController::updatePort(QNEOutputPort *port)
{
    if (!port) {
        return;
    }

    GraphicElement *elm = port->graphicElement();
    LogicElement *logic = nullptr;
    int portIndex = 0;

    if (elm->elementType() == ElementType::IC) {
        IC *ic = dynamic_cast<IC *>(elm);
        logic = m_elmMapping->icMapping(ic)->output(port->index());
    } else {
        logic = elm->logic();
        portIndex = port->index();
    }

    if (logic->isValid()) {
        port->setStatus(static_cast<Status>(logic->outputValue(portIndex)));
    } else {
        port->setStatus(Status::Invalid);
    }
}

void SimulationController::updatePort(QNEInputPort *port)
{
    GraphicElement *elm = port->graphicElement();
    LogicElement *logic = elm->logic();
    int portIndex = port->index();

    if (logic->isValid()) {
        port->setStatus(static_cast<Status>(logic->inputValue(portIndex)));
    } else {
        port->setStatus(Status::Invalid);
    }

    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}

void SimulationController::restart()
{
    m_initialized = false;
}

bool SimulationController::canRun() const
{
    return m_initialized;
}

bool SimulationController::isRunning()
{
    return m_simulationTimer.isActive();
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

bool SimulationController::initialize()
{
    qCDebug(two) << tr("GENERATING SIMULATION LAYER.");
    auto elements = m_scene->elements();
    qCDebug(zero) << tr("Elements read:") << elements.size();
    if (elements.empty()) {
        return false;
    }
    qCDebug(two) << tr("Recreating mapping for simulation.");
    m_elmMapping = std::make_unique<ElementMapping>(elements);
    if (!m_elmMapping->canInitialize()) {
        qCDebug(zero) << tr("Cannot initialize simulation.");
        return false;
    }
    qCDebug(two) << tr("Can initialize.");
    m_elmMapping->initialize();
    qCDebug(two) << tr("Sorting.");
    m_elmMapping->sort();
    m_initialized = true;
    qCDebug(zero) << tr("Finished simulation layer.");
    return true;
}
