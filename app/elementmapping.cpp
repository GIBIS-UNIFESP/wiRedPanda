// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementmapping.h"

#include "common.h"
#include "elementfactory.h"
#include "graphicelement.h"
#include "ic.h"
#include "qneconnection.h"
#include "qneport.h"

#include <QDebug>

ElementMapping::ElementMapping(const QVector<GraphicElement *> &elements)
    : m_elements(elements)
{
    qCDebug(three) << "Generate Map.";
    qDebug() << "CRASH_DEBUG: ElementMapping constructor - elements.size():" << elements.size();
    for (int i = 0; i < elements.size(); ++i) {
        auto *elm = elements[i];
        qDebug() << "CRASH_DEBUG: Element" << i << ":" << elm << "Type:" << static_cast<int>(elm->elementType());
    }
    generateMap();
    qCDebug(three) << "Connect.";
    qDebug() << "CRASH_DEBUG: About to connectElements()";
    connectElements();
    qDebug() << "CRASH_DEBUG: connectElements() completed";
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
    qDebug() << "CRASH_DEBUG: applyConnection - Element:" << elm << "Type:" << static_cast<int>(elm->elementType()) << "InputPort:" << inputPort;
    
    LogicElement *currentLogic;
    int inputIndex = 0;

    if (elm->elementType() == ElementType::IC) {
        auto *ic = qobject_cast<IC *>(elm);
        currentLogic = ic->inputLogic(inputPort->index());
        qDebug() << "CRASH_DEBUG: IC currentLogic:" << currentLogic;
    } else {
        currentLogic = elm->logic();
        inputIndex = inputPort->index();
        qDebug() << "CRASH_DEBUG: Element currentLogic:" << currentLogic << "inputIndex:" << inputIndex;
    }

    if (!currentLogic) {
        qDebug() << "CRASH_DEBUG: currentLogic is NULL for element:" << elm;
        return;
    }

    const auto connections = inputPort->connections();
    qDebug() << "CRASH_DEBUG: connections.size():" << connections.size();

    if ((connections.size() == 0) && !inputPort->isRequired()) {
        auto *predecessorLogic = (inputPort->defaultValue() == Status::Active) ? &m_globalVCC : &m_globalGND;
        qDebug() << "CRASH_DEBUG: Connecting to global" << (predecessorLogic == &m_globalVCC ? "VCC" : "GND") << "predecessorLogic:" << predecessorLogic;
        currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
    }

    if (connections.size() == 1) {
        auto *connection = connections.constFirst();
        qDebug() << "CRASH_DEBUG: Connection:" << connection << "Type:" << connection->type();
        
        if (auto *outputPort = connection->startPort()) {
            qDebug() << "CRASH_DEBUG: OutputPort:" << outputPort;
            
            if (auto *predecessorElement = outputPort->graphicElement()) {
                qDebug() << "CRASH_DEBUG: PredecessorElement:" << predecessorElement << "Type:" << static_cast<int>(predecessorElement->elementType());
                
                if (predecessorElement->elementType() == ElementType::IC) {
                    auto *ic = qobject_cast<IC *>(predecessorElement);
                    auto *predecessorLogic = ic->outputLogic(outputPort->index());
                    qDebug() << "CRASH_DEBUG: IC predecessorLogic:" << predecessorLogic;
                    if (predecessorLogic) {
                        currentLogic->connectPredecessor(inputIndex, predecessorLogic, 0);
                    } else {
                        qDebug() << "CRASH_DEBUG: IC predecessorLogic is NULL!";
                    }
                } else {
                    auto *predecessorLogic = predecessorElement->logic();
                    qDebug() << "CRASH_DEBUG: Element predecessorLogic:" << predecessorLogic << "outputPort->index():" << outputPort->index();
                    if (predecessorLogic) {
                        currentLogic->connectPredecessor(inputIndex, predecessorLogic, outputPort->index());
                    } else {
                        qDebug() << "CRASH_DEBUG: Element predecessorLogic is NULL!";
                    }
                }
            } else {
                qDebug() << "CRASH_DEBUG: predecessorElement is NULL!";
            }
        } else {
            qDebug() << "CRASH_DEBUG: outputPort is NULL!";
        }
    }
    
    qDebug() << "CRASH_DEBUG: applyConnection completed for element:" << elm;
}

void ElementMapping::sort()
{
    sortLogicElements();
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

const QVector<std::shared_ptr<LogicElement>> &ElementMapping::logicElms() const
{
    return m_logicElms;
}
