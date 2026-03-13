// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/ElementMapping.h"

#include "App/Core/Common.h"
#include "App/Core/Priorities.h"
#include "App/Element/GraphicElement.h"
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
        m_logicElms.append(elm->createLogicElements());
        elm->bindPorts();
    }
}

void ElementMapping::connectElements()
{
    for (auto *elm : std::as_const(m_elements)) {
        for (auto *inputPort : elm->inputs()) {
            applyConnection(inputPort);
        }
    }
}

void ElementMapping::applyConnection(QNEInputPort *inputPort)
{
    auto *currentLogic = inputPort->logic();
    if (!currentLogic) {
        return;
    }

    int inputIndex = inputPort->logicIndex();

    const auto connections = inputPort->connections();

    if ((connections.size() == 0) && !inputPort->isRequired()) {
        auto *predecessorLogic = (inputPort->defaultValue() == Status::Active) ? &m_globalVCC : &m_globalGND;
        currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
    }

    if (connections.size() == 1) {
        auto *connection = connections.constFirst();
        if (!connection) {
            return;
        }
        if (auto *outputPort = connection->startPort()) {
            auto *predecessorLogic = outputPort->logic();
            if (!predecessorLogic) {
                return;
            }
            int outputIndex = outputPort->logicIndex();
            currentLogic->connectPredecessor(inputIndex, predecessorLogic, outputIndex);
        }
    }
}

int ElementMapping::priority(const LogicElement *logic) const
{
    return m_priorities.value(logic, -1);
}

bool ElementMapping::isInFeedbackLoop(const LogicElement *logic) const
{
    return m_feedbackNodes.contains(logic);
}

bool ElementMapping::hasFeedbackElements() const
{
    return !m_feedbackNodes.isEmpty();
}

void ElementMapping::sort()
{
    sortLogicElements();
}

void ElementMapping::sortLogicElements()
{
    QHash<LogicElement *, QVector<LogicElement *>> successors;
    QVector<LogicElement *> rawPtrs;
    rawPtrs.reserve(m_logicElms.size());

    for (const auto &logic : std::as_const(m_logicElms)) {
        if (!logic) {
            continue;
        }
        rawPtrs.append(logic.get());
        for (const auto &pair : logic->inputPairs()) {
            if (pair.logic && !successors[pair.logic].contains(logic.get())) {
                successors[pair.logic].append(logic.get());
            }
        }
    }

    QHash<LogicElement *, int> priorities;
    calculatePriorities(rawPtrs, successors, priorities);
    const auto feedbackElements = findFeedbackNodes(rawPtrs, successors);

    m_priorities.clear();
    m_feedbackNodes.clear();
    for (const auto &logic : std::as_const(m_logicElms)) {
        m_priorities[logic.get()] = priorities.value(logic.get(), -1);
        if (feedbackElements.contains(logic.get())) {
            m_feedbackNodes.insert(logic.get());
        }
    }

    std::stable_sort(m_logicElms.begin(), m_logicElms.end(), [this](const auto &logic1, const auto &logic2) {
        return m_priorities.value(logic1.get(), -1) > m_priorities.value(logic2.get(), -1);
    });
}

const QVector<std::shared_ptr<LogicElement>> &ElementMapping::logicElms() const
{
    return m_logicElms;
}
