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

    for (auto *clock : qAsConst(m_clocks)) {
        clock->updateClock();
    }

    for (auto *inputElm : qAsConst(m_inputs)) {
        inputElm->updateOutputs();
    }

    for (auto logic : m_elmMapping->logicElms()) {
        logic->updateLogic();
    }

    for (auto *connection : qAsConst(m_connections)) {
        updatePort(connection->start());
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

    qCDebug(two) << tr("GENERATING SIMULATION LAYER.");

    QVector<GraphicElement *> elements;
    const auto items = m_scene->items(Qt::SortOrder(-1));

    if (items.size() == 1) {
        return false;
    }

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

    qCDebug(zero) << tr("Elements read:") << elements.size();
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
