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
    connectWirelessElements();
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

void ElementMapping::connectWirelessElements()
{
    // Build a map from channel label to the Tx node's LogicElement.
    // If two Tx nodes share the same label the first registered wins and a warning is emitted.
    QHash<QString, LogicElement *> txMap;
    for (auto *elm : std::as_const(m_elements)) {
        if (elm->wirelessMode() != WirelessMode::Tx || elm->label().isEmpty() || !elm->logic()) {
            continue;
        }
        if (txMap.contains(elm->label())) {
            qCWarning(zero) << "Duplicate wireless Tx label:" << elm->label() << "— second transmitter ignored.";
        } else {
            txMap.insert(elm->label(), elm->logic());
        }
    }

    // Wire each Rx node's LogicNode input to the matching Tx LogicNode output.
    // connectPredecessor() overwrites whatever applyConnection() set (GND fallback via
    // setRequired(false)), so the topological sort will see the true wireless dependency.
    for (auto *elm : std::as_const(m_elements)) {
        if (elm->wirelessMode() != WirelessMode::Rx || elm->label().isEmpty() || !elm->logic()) {
            continue;
        }
        if (auto *txLogic = txMap.value(elm->label(), nullptr)) {
            elm->logic()->connectPredecessor(0, txLogic, 0);
        }
        // No matching Tx: applyConnection() already wired this port to GND (setRequired(false)),
        // so the element is valid and outputs logic 0.
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
    // Initialize successor storage for the global VCC/GND sources — they are
    // not in m_logicElms but can be predecessors of many elements.
    m_globalVCC.initSuccessors(m_globalVCC.outputSize());
    m_globalGND.initSuccessors(m_globalGND.outputSize());

    QHash<LogicElement *, QVector<LogicElement *>> successors;
    QVector<LogicElement *> rawPtrs;
    rawPtrs.reserve(m_logicElms.size());

    // First pass: initialize successor storage for every element so that
    // addSuccessor() in the second pass never indexes an un-sized vector.
    for (const auto &logic : std::as_const(m_logicElms)) {
        if (logic) {
            logic->initSuccessors(logic->outputSize());
        }
    }

    // Second pass: build predecessor-to-successor mappings.
    for (const auto &logic : std::as_const(m_logicElms)) {
        if (!logic) {
            continue;
        }
        rawPtrs.append(logic.get());

        for (int inIdx = 0; inIdx < logic->inputPairs().size(); ++inIdx) {
            const auto &pair = logic->inputPairs()[inIdx];
            if (!pair.logic) {
                continue;
            }

            // Element-level successor map (for topological sort / priority calculation).
            if (!successors[pair.logic].contains(logic.get())) {
                successors[pair.logic].append(logic.get());
            }

            // Port-granular successor list (for temporal event scheduling).
            // The predecessor may be an external element (e.g., IC-internal
            // globalVCC/GND) not in m_logicElms; lazily initialize its
            // successor storage if it hasn't been done yet.
            if (pair.logic->successorGroupCount() == 0) {
                pair.logic->initSuccessors(pair.logic->outputSize());
            }
            pair.logic->addSuccessor(pair.port, {logic.get(), inIdx});
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
