// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulation.h"

#include "clock.h"
#include "common.h"
#include "elementmapping.h"
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

    if (m_timer.isActive()) {
        for (auto *clock : qAsConst(m_clocks)) {
            clock->updateClock();
        }
    }

    for (auto *inputElm : qAsConst(m_inputs)) {
        inputElm->updateOutputs();
    }

    for (auto logic : m_elmMapping->logicElms()) {
        logic->updateLogic();
        logic->setTemporalSimulationIsOn(m_temporalSimulation);
    }

    for (auto *connection : qAsConst(m_connections)) {
        updatePort(connection->startPort());
    }

    for (auto *outputElm : qAsConst(m_outputs)) {
        for (auto *inputPort : outputElm->inputs()) {
            updatePort(inputPort);
        }
    }
}

void Simulation::updatePort(QNEOutputPort *port)
{
    if (!port) {
        return;
    }

    auto *elm = port->graphicElement();

    if (elm->elementType() == ElementType::IC) {
        auto *logic = qobject_cast<IC *>(elm)->outputLogic(port->index());
        port->setStatus(logic->isValid() ? static_cast<Status>(logic->outputValue(0)) : Status::Invalid);
    } else {
        auto *logic = elm->logic();
        port->setStatus(logic->isValid() ? static_cast<Status>(logic->outputValue(port->index())) : Status::Invalid);
    }
}

void Simulation::updatePort(QNEInputPort *port)
{
    auto *elm = port->graphicElement();
    auto *logic = elm->logic();

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
        initialize();
    }

    m_timer.start();
    m_scene->mute(false);
    qCDebug(zero) << tr("Simulation started.");
}

bool Simulation::initialize()
{
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_connections.clear();

    QVector<GraphicElement *> elements;
    const auto items = m_scene->items();

    if (items.size() == 1) {
        return false;
    }

    qCDebug(two) << tr("GENERATING SIMULATION LAYER.");

    for (auto *item : items) {
        if (item->type() == QNEConnection::Type) {
            m_connections.append(qgraphicsitem_cast<QNEConnection *>(item));
        }

        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);
            elements.append(element);

            if (element->elementType() == ElementType::Clock) {
                m_clocks.append(qobject_cast<Clock *>(element));
                m_clocks.constLast()->resetClock();
            }

            if (element->elementGroup() == ElementGroup::Input) {
                m_inputs.append(qobject_cast<GraphicElementInput *>(element));
            }

            if (element->elementGroup() == ElementGroup::Output) {
                m_outputs.append(element);
            }
        }
    }

    std::sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
        return a->priority() > b->priority();
    });

    qCDebug(zero) << tr("Elements read: ") << elements.size();

    if (elements.empty()) {
        return false;
    }

    qCDebug(two) << tr("Recreating mapping for simulation.");
    m_elmMapping = std::make_unique<ElementMapping>(elements);

    qCDebug(two) << tr("Sorting.");
    m_elmMapping->sort();

    m_initialized = true;

    qCDebug(zero) << tr("Finished simulation layer.");
    return true;
}

void Simulation::setTemporalSimulation(bool isOn)
{
    m_temporalSimulation = isOn;
}

bool Simulation::isTemporalSimulation()
{
    return m_temporalSimulation;
}
