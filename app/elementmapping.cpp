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

    if ((connections.size() == 0) && !inputPort->isRequired()) {
        auto *predecessorLogic = (inputPort->defaultValue() == Status::Active) ? &m_globalVCC : &m_globalGND;
        currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
    }

    if (connections.size() == 1) {
        if (auto *outputPort = connections.constFirst()->startPort()) {
            if (auto *predecessorElement = outputPort->graphicElement()) {
                if (predecessorElement->elementType() == ElementType::IC) {
                    auto *predecessorLogic = qobject_cast<IC *>(predecessorElement)->outputLogic(outputPort->index());
                    currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
                } else {
                    currentLogic->connectPredecessor(inputIndex, predecessorElement->logic(), outputPort->index());
                }
            }
        }
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
    qCDebug(zero) << "CONVERGENCE DEBUG: Detecting feedback loops...";
    qCDebug(zero) << "CONVERGENCE DEBUG: Total logic elements:" << m_logicElms.size();
    
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
    
    qCDebug(zero) << "CONVERGENCE DEBUG: Found" << m_feedbackGroups.size() << "feedback groups";
    for (int i = 0; i < m_feedbackGroups.size(); ++i) {
        qCDebug(zero) << "CONVERGENCE DEBUG: Feedback group" << i << "has" << m_feedbackGroups[i].size() << "elements";
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
            if (logic->getInputSource(i) == current) {
                if (recursionStack.contains(logic.get())) {
                    // Found cycle! Mark all elements in cycle as feedback dependent
                    qCDebug(zero) << "CONVERGENCE DEBUG: CYCLE DETECTED!";
                    markFeedbackCycle(currentPath, logic.get());
                } else if (!visited.contains(logic.get())) {
                    findCycles(logic.get(), visited, recursionStack, currentPath);
                }
                break;
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
