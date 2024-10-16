
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



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
    for (auto *elm : qAsConst(m_elements)) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = qobject_cast<IC *>(elm);
            m_logicElms.append(ic->generateMap()->m_logicElms);
            continue;
        }

        generateLogic(elm);
    }
}

void ElementMapping::generateLogic(GraphicElement *elm)
{
    auto logic = ElementFactory::buildLogicElement(elm);
    elm->setLogic(logic.get());
    m_logicElms.append(logic);
}

void ElementMapping::connectElements()
{
    for (auto *elm : qAsConst(m_elements)) {
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
    validateElements();
}

void ElementMapping::sortLogicElements()
{
    for (auto logic : qAsConst(m_logicElms)) {
        logic->calculatePriority();
    }

    std::sort(m_logicElms.begin(), m_logicElms.end(), [](const auto &logic1, const auto &logic2) {
        return *logic1 > *logic2;
    });
}

void ElementMapping::validateElements()
{
    for (auto logic : qAsConst(m_logicElms)) {
        logic->validate();
    }
}

const QVector<std::shared_ptr<LogicElement>> &ElementMapping::logicElms() const
{
    return m_logicElms;
}
