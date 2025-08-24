// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementmapping.h"

#include "common.h"
#include "elementfactory.h"
#include "graphicelement.h"
#include "ic.h"
#include "qneconnection.h"
#include "qneport.h"

ElementMapping::ElementMapping(const QVector<GraphicElement *> &elements)
    : m_elements(elements)
{
    qCDebug(three) << "Generate Map.";
    generateMap();
    qCDebug(three) << "Connect.";
    connectElements();
}

ElementMapping::~ElementMapping()
{
    m_globalGND.clearSucessors();
    m_globalVCC.clearSucessors();
}

void ElementMapping::generateMap()
{
    for (auto *elm : std::as_const(m_elements)) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);
            m_logicElms.append(ic->generateMap());
            continue;
        }

        generateLogic(elm);
    }
}

void ElementMapping::generateLogic(GraphicElement *elm)
{
    m_logicElms.append(ElementFactory::buildLogicElement(elm));
    elm->setLogic(m_logicElms.last().get());
}

void ElementMapping::connectElements()
{
    for (auto *elm : std::as_const(m_elements)) {
        for (auto *inputPort : elm->inputs()) {
            applyConnection(elm, inputPort);
        }
    }
}

void ElementMapping::applyConnection(GraphicElement *elm, QNEInputPort *inputPort)
{
    LogicElement *currentLogic;
    int inputIndex = 0;

    if (elm->elementType() == ElementType::IC) {
        auto *ic = qobject_cast<IC *>(elm);
        currentLogic = ic->inputLogic(inputPort->index());
    } else {
        currentLogic = elm->logic();
        inputIndex = inputPort->index();
    }

    const auto connections = inputPort->connections();
    
    // DEBUG: Trace connection mapping for all inputs
    qDebug() << "CONNECTION MAPPING DEBUG: Processing input" << inputIndex << "of element" << (void*)elm << "(" << elm->objectName() << ")";
    qDebug() << "CONNECTION MAPPING DEBUG: Input port index:" << inputPort->index() << "name:" << inputPort->name() << "connections:" << connections.size();
    qDebug() << "CONNECTION MAPPING DEBUG: Port required:" << inputPort->isRequired() << "default:" << (int)inputPort->defaultValue();

    if ((connections.size() == 0) && !inputPort->isRequired()) {
        auto *predecessorLogic = (inputPort->defaultValue() == Status::Active) ? &m_globalVCC : &m_globalGND;
        qDebug() << "CONNECTION MAPPING DEBUG: Connecting input" << inputIndex << "to" << (predecessorLogic == &m_globalVCC ? "VCC" : "GND");
        currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
    }

    if (connections.size() == 1) {
        qDebug() << "CONNECTION MAPPING DEBUG: Processing single connection for input" << inputIndex;
        if (auto *outputPort = connections.constFirst()->startPort()) {
            qDebug() << "CONNECTION MAPPING DEBUG: Found start port:" << (void*)outputPort << "from element" << (void*)outputPort->graphicElement();
            if (auto *predecessorElement = outputPort->graphicElement()) {
                if (predecessorElement->elementType() == ElementType::IC) {
                    auto *predecessorLogic = qobject_cast<IC *>(predecessorElement)->outputLogic(outputPort->index());
                    qDebug() << "CONNECTION MAPPING DEBUG: Connecting IC input" << inputIndex << "to predecessor" << (void*)predecessorLogic << "port 0";
                    currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
                } else {
                    qDebug() << "CONNECTION MAPPING DEBUG: Connecting regular input" << inputIndex << "to predecessor" << (void*)predecessorElement->logic() << "port" << outputPort->index();
                    currentLogic->connectPredecessor(inputIndex, predecessorElement->logic(), outputPort->index());
                }
            } else {
                qDebug() << "CONNECTION MAPPING DEBUG: ERROR - No predecessor element for input" << inputIndex;
            }
        } else {
            qDebug() << "CONNECTION MAPPING DEBUG: ERROR - No start port for input" << inputIndex;
        }
    } else if (connections.size() > 1) {
        qDebug() << "CONNECTION MAPPING DEBUG: WARNING - Multiple connections on input" << inputIndex << "- count:" << connections.size();
    } else {
        qDebug() << "CONNECTION MAPPING DEBUG: No connections on input" << inputIndex;
    }
}

void ElementMapping::sort()
{
    sortLogicElements();
    detectFeedbackLoops();  // Add feedback loop detection
    validateElements();
}

void ElementMapping::sortLogicElements()
{
    for (auto &logic : std::as_const(m_logicElms)) {
        logic->calculatePriority();
    }

    std::sort(m_logicElms.begin(), m_logicElms.end(), [](const auto &logic1, const auto &logic2) {
        return logic1->priority() > logic2->priority();
    });
}

void ElementMapping::validateElements()
{
    for (auto &logic : std::as_const(m_logicElms)) {
        logic->validate();
    }
}

void ElementMapping::detectFeedbackLoops()
{
    qDebug() << "CONVERGENCE DEBUG: Detecting feedback loops...";
    qDebug() << "CONVERGENCE DEBUG: Total logic elements:" << m_logicElms.size();
    
    // Clear any existing feedback groups
    m_feedbackGroups.clear();
    
    // Mark all elements initially as combinational
    for (auto &logic : m_logicElms) {
        logic->setFeedbackDependent(false);
    }
    
    // Detect cycles using DFS
    QSet<LogicElement*> visited;
    QSet<LogicElement*> recursionStack;
    QVector<LogicElement*> currentPath;
    
    for (auto &logic : m_logicElms) {
        if (!visited.contains(logic.get())) {
            findCycles(logic.get(), visited, recursionStack, currentPath);
        }
    }
    
    qDebug() << "CONVERGENCE DEBUG: Found" << m_feedbackGroups.size() << "feedback groups";
    for (int i = 0; i < m_feedbackGroups.size(); ++i) {
        qDebug() << "CONVERGENCE DEBUG: Feedback group" << i << "has" << m_feedbackGroups[i].size() << "elements";
    }
}

void ElementMapping::findCycles(LogicElement *current, QSet<LogicElement*> &visited,
                                QSet<LogicElement*> &recursionStack, QVector<LogicElement*> &currentPath)
{
    visited.insert(current);
    recursionStack.insert(current);
    currentPath.append(current);
    
    // Check all successors (elements that depend on this element)
    for (auto &logic : m_logicElms) {
        // Check if 'logic' depends on 'current' 
        for (int i = 0; i < logic->inputCount(); ++i) {
            auto *inputSource = logic->getInputSource(i);
            if (inputSource == current) {
                qDebug() << "CONVERGENCE DEBUG: Found dependency:" << (void*)current << "->" << (void*)logic.get() << "at input" << i;
                if (recursionStack.contains(logic.get())) {
                    // Found cycle! Mark all elements in cycle as feedback dependent
                    qDebug() << "CONVERGENCE DEBUG: CYCLE DETECTED! Path size:" << currentPath.size();
                    markFeedbackCycle(currentPath, logic.get());
                } else if (!visited.contains(logic.get())) {
                    findCycles(logic.get(), visited, recursionStack, currentPath);
                }
                break;
            } else if (inputSource != nullptr) {
                // Debug: Show what the input source actually is when it's not null
                qDebug() << "CONVERGENCE DEBUG: Input" << i << "of" << (void*)logic.get() << "connected to" << (void*)inputSource << "(not" << (void*)current << ")";
            }
        }
    }
    
    recursionStack.remove(current);
    currentPath.removeLast();
}

void ElementMapping::markFeedbackCycle(const QVector<LogicElement*> &path, LogicElement *cycleStart)
{
    // Find the start of the cycle in the current path
    int cycleStartIndex = path.indexOf(cycleStart);
    if (cycleStartIndex == -1) return;
    
    // Create a feedback group for this cycle
    QVector<std::shared_ptr<LogicElement>> feedbackGroup;
    
    // Add all elements in the cycle to the feedback group
    for (int i = cycleStartIndex; i < path.size(); ++i) {
        path[i]->setFeedbackDependent(true);
        
        // Find the shared_ptr for this element
        for (auto &logic : m_logicElms) {
            if (logic.get() == path[i]) {
                feedbackGroup.append(logic);
                break;
            }
        }
    }
    
    // Also add the cycle start element (completes the cycle)
    cycleStart->setFeedbackDependent(true);
    bool cycleStartAlreadyAdded = false;
    for (const auto &element : feedbackGroup) {
        if (element.get() == cycleStart) {
            cycleStartAlreadyAdded = true;
            break;
        }
    }
    
    if (!cycleStartAlreadyAdded) {
        for (auto &logic : m_logicElms) {
            if (logic.get() == cycleStart) {
                feedbackGroup.append(logic);
                break;
            }
        }
    }
    
    if (!feedbackGroup.isEmpty()) {
        m_feedbackGroups.append(feedbackGroup);
    }
}

const QVector<QVector<std::shared_ptr<LogicElement>>>& ElementMapping::feedbackGroups() const
{
    return m_feedbackGroups;
}

const QVector<std::shared_ptr<LogicElement>> &ElementMapping::logicElms() const
{
    return m_logicElms;
}
