// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulation.h"

#include "buzzer.h"
#include "clock.h"
#include "common.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "qneconnection.h"
#include "scene.h"

#include <QGraphicsView>

using namespace std::chrono_literals;

Simulation::Simulation(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    m_timer.setInterval(1ms);
    connect(&m_timer, &QTimer::timeout, this, &Simulation::update);
}

void Simulation::update()
{
    if (!m_initialized && !initialize()) {
        return;
    }

    for (auto *clock : qAsConst(m_clocks)) {
        if (!clock) {
            continue;
        };

        Clock::reset ? clock->resetClock() : clock->updateClock();
    }

    Clock::reset = false;

    for (auto *input : qAsConst(m_inputs)) {
        input->updatePortsOutputs();
    }

    // FIXME: This can easily cause crashes when using the Undo command to delete elements
    for (auto *logic : m_elmMapping->logicElms()) {
        logic->updateLogic();
    }

    updateScene();
}

void Simulation::updateScene()
{
    for (auto *item : qAsConst(m_items)) {
        if (item->type() == QNEConnection::Type) {
            auto *connection = qgraphicsitem_cast<QNEConnection *>(item);
            updatePort(connection->start());
            continue;
        }

        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);

            if (element && (element->elementGroup() == ElementGroup::Output)) {
                for (auto *port : element->inputs()) {
                    updatePort(port);
                }
            }
        }
    }
}

void Simulation::updatePort(QNEOutputPort *port)
{
    if (!port) {
        return;
    }

    GraphicElement *elm = port->graphicElement();
    LogicElement *logic = nullptr;
    int portIndex = 0;

    if (elm->elementType() == ElementType::IC) {
        IC *ic = qobject_cast<IC *>(elm);
        logic = ic->icOutput(port->index());
    } else {
        logic = elm->logic();
        portIndex = port->index();
    }

    port->setStatus(logic->isValid() ? static_cast<Status>(logic->outputValue(portIndex)) : Status::Invalid);
}

void Simulation::updatePort(QNEInputPort *port)
{
    GraphicElement *elm = port->graphicElement();
    LogicElement *logic = elm->logic();

    port->setStatus(logic->isValid() ? static_cast<Status>(logic->inputValue(port->index())) : Status::Invalid);

    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}

void Simulation::restart()
{
    m_initialized = false;
}

bool Simulation::isRunning()
{
    return m_timer.isActive();
}

void Simulation::stop()
{
    m_timer.stop();
    m_scene->mute(true);
}

void Simulation::start()
{
    qCDebug(zero) << tr("Starting simulation.");
    if (!m_initialized) {
        Clock::reset = true;
        initialize();
    }

    m_timer.start();
    m_scene->mute(false);
    qCDebug(zero) << tr("Simulation started.");
}

bool Simulation::initialize()
{
    m_clocks.clear();
    m_inputs.clear();
    m_items.clear();

    qCDebug(two) << tr("GENERATING SIMULATION LAYER.");
    const auto elements = m_scene->elements();

    qCDebug(zero) << tr("Elements read:") << elements.size();
    if (elements.empty()) {
        return false;
    }

    for (auto *elm : elements) {
        if (elm->elementType() == ElementType::Clock) {
            m_clocks.append(qobject_cast<Clock *>(elm));
        }

        if (elm->elementGroup() == ElementGroup::Input) {
            m_inputs.append(qobject_cast<GraphicElementInput *>(elm));
        }
    }

    m_items = m_scene->items(Qt::SortOrder(-1));

    qCDebug(two) << tr("Recreating mapping for simulation.");
    m_elmMapping = std::make_unique<ElementMapping>(elements);

    qCDebug(two) << tr("Sorting.");
    m_elmMapping->sort();
    m_initialized = true;

    qCDebug(zero) << tr("Finished simulation layer.");
    return true;
}
