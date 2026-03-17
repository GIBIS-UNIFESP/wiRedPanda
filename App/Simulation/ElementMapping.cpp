// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/ElementMapping.h"

#include "App/Core/Common.h"
#include "App/Core/Priorities.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"

ElementMapping::ElementMapping(const QVector<GraphicElement *> &elements,
                               std::shared_ptr<LogicSource> gnd,
                               std::shared_ptr<LogicSource> vcc)
    : m_globalGND(gnd ? std::move(gnd) : std::make_shared<LogicSource>(false))
    , m_globalVCC(vcc ? std::move(vcc) : std::make_shared<LogicSource>(true))
    , m_elements(elements)
{
    // If we created GND ourselves, we're the sole owner (use_count == 1).
    // If shared from a parent mapping, the parent also holds a reference.
    const bool ownsGlobalSources = (m_globalGND.use_count() == 1);

    qCDebug(three) << "Generate Map.";
    generateMap();
    // Only the top-level mapping owns the GND/VCC elements in m_logicElms.
    // Nested IC mappings share the same instances via the gnd/vcc parameters.
    if (ownsGlobalSources) {
        m_logicElms.append(m_globalGND);
        m_logicElms.append(m_globalVCC);
    }
    qCDebug(three) << "Connect.";
    connectElements();
    connectWirelessElements();
}

void ElementMapping::generateMap()
{
    for (auto *elm : std::as_const(m_elements)) {
        // Pass shared GND/VCC to IC elements so the entire graph uses one pair.
        if (elm->elementType() == ElementType::IC) {
            if (auto *ic = dynamic_cast<IC *>(elm)) {
                ic->setGlobalSources(m_globalGND, m_globalVCC);
            }
        }
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
    // setRequired(false)), so the successor graph will see the true wireless dependency.
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
        auto *predecessorLogic = (inputPort->defaultValue() == Status::Active) ? m_globalVCC.get() : m_globalGND.get();
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

bool ElementMapping::isInFeedbackLoop(const LogicElement *logic) const
{
    return m_feedbackNodes.contains(logic);
}

bool ElementMapping::hasFeedbackElements() const
{
    return !m_feedbackNodes.isEmpty();
}

void ElementMapping::buildGraph()
{
    buildSuccessorGraph();
}

void ElementMapping::buildSuccessorGraph()
{
    QHash<LogicElement *, QVector<LogicElement *>> successors;
    QVector<LogicElement *> rawPtrs;
    rawPtrs.reserve(m_logicElms.size());

    // First pass: initialize successor storage for every element.
    for (const auto &logic : std::as_const(m_logicElms)) {
        if (logic) {
            logic->initSuccessors(logic->outputSize());
        }
    }

    // Second pass: build element-level and port-granular successor maps.
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

            // Element-level successor map (for feedback detection).
            if (!successors[pair.logic].contains(logic.get())) {
                successors[pair.logic].append(logic.get());
            }

            // Port-granular successor list (for event-driven scheduling).
            if (pair.logic->successorGroupCount() == 0) {
                pair.logic->initSuccessors(pair.logic->outputSize());
            }
            pair.logic->addSuccessor(pair.port, {logic.get(), inIdx});
        }
    }

    const auto feedbackElements = findFeedbackNodes(rawPtrs, successors);

    m_feedbackNodes.clear();
    for (const auto &logic : std::as_const(m_logicElms)) {
        if (feedbackElements.contains(logic.get())) {
            m_feedbackNodes.insert(logic.get());
        }
    }
}

const QVector<std::shared_ptr<LogicElement>> &ElementMapping::logicElms() const
{
    return m_logicElms;
}

