// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/ElementMapping.h"

#include "App/Core/Common.h"
#include "App/Element/ElementMetadata.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Element/LogicElements/LogicNone.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"

ElementMapping::ElementMapping(const QVector<GraphicElement *> &elements)
    : m_elements(elements)
{
    qCDebug(three) << "Generate Map.";
    generateMap();
    qCDebug(three) << "Connect.";
    connectElements();
}

void ElementMapping::generateMap()
{
    for (auto *elm : std::as_const(m_elements)) {
        auto logicElements = elm->getLogicElementsForMapping();
        if (logicElements.size() > 1) {
            // Multi-logic element (like IC): port logic is set by getLogicElementsForMapping()
            m_logicElms.append(logicElements);
        } else {
            // Primitive elements (gates, latches, etc.) get one logic node each;
            // generateLogic() creates the node and links it back to the graphic element.
            generateLogic(elm);
        }
    }
}

void ElementMapping::generateLogic(GraphicElement *elm)
{
    const auto &meta = ElementMetadataRegistry::metadata(elm->elementType());
    auto logicElm = meta.logicCreator ? meta.logicCreator(elm) : std::make_shared<LogicNone>();
    m_logicElms.append(std::move(logicElm));
    auto *logic = m_logicElms.last().get();
    elm->setLogic(logic);

    for (int i = 0; i < elm->inputSize(); ++i) {
        elm->inputPort(i)->setPortLogic(logic, i);
    }
    for (int i = 0; i < elm->outputSize(); ++i) {
        elm->outputPort(i)->setPortLogic(logic, i);
    }
}

void ElementMapping::connectElements()
{
    // Walk every graphic element's input ports and wire the corresponding logic
    // predecessors.  Output ports don't need iteration here because a connection
    // is described entirely from the consumer (input-port) side.
    for (auto *elm : std::as_const(m_elements)) {
        for (auto *inputPort : elm->inputs()) {
            applyConnection(inputPort);
        }
    }
}

void ElementMapping::applyConnection(QNEInputPort *inputPort)
{
    auto *currentLogic = inputPort->logic();
    int inputIndex = inputPort->logicIndex();

    const auto connections = inputPort->connections();

    // Unconnected optional inputs fall back to the global constant VCC or GND
    // node so that the logic element always has a valid predecessor and
    // validate() can mark it as ready.
    if ((connections.size() == 0) && !inputPort->isRequired()) {
        auto *predecessorLogic = (inputPort->defaultValue() == Status::Active) ? &m_globalVCC : &m_globalGND;
        currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
    }

    // Each input port can have at most one driver (multiple connections on a
    // single input are forbidden by the UI); the constFirst() call is safe here.
    if (connections.size() == 1) {
        auto *connection = connections.constFirst();
        if (!connection) {
            return;
        }
        if (auto *outputPort = connection->startPort()) {
            auto *predecessorLogic = outputPort->logic();
            int outputIndex = outputPort->logicIndex();
            currentLogic->connectPredecessor(inputIndex, predecessorLogic, outputIndex);
        }
    }
}

void ElementMapping::sort()
{
    // Two-step finalisation: first assign priorities so every node knows its
    // depth, then sort and mark nodes as valid/invalid for simulation.
    sortLogicElements();
    validateElements();
}

void ElementMapping::sortLogicElements()
{
    QHash<LogicElement *, QVector<LogicElement *>> successors;
    for (const auto &logic : std::as_const(m_logicElms)) {
        for (const auto &pair : logic->inputPairs()) {
            if (pair.logic && !successors[pair.logic].contains(logic.get())) {
                successors[pair.logic].append(logic.get());
            }
        }
    }

    for (auto &logic : std::as_const(m_logicElms)) {
        logic->calculatePriority(successors);
    }

    // Higher priority = closer to the inputs; sort descending so inputs are
    // evaluated before the gates that depend on them.
    std::stable_sort(m_logicElms.begin(), m_logicElms.end(), [](const auto &logic1, const auto &logic2) {
        return logic1->priority() > logic2->priority();
    });
}

void ElementMapping::validateElements()
{
    for (auto &logic : std::as_const(m_logicElms)) {
        logic->validate();
    }
}

const QVector<std::shared_ptr<LogicElement>> &ElementMapping::logicElms() const
{
    return m_logicElms;
}

