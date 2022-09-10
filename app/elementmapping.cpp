// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
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
    qCDebug(three) << tr("Generate Map.");
    generateMap();
    qCDebug(three) << tr("Connect.");
    connectElements();
}

ElementMapping::~ElementMapping()
{
    clear();
}

void ElementMapping::clear()
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
    auto logicElm = ElementFactory::buildLogicElement(elm);
    elm->setLogic(logicElm.get());
    m_logicElms.append(logicElm);
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
    LogicElement *currentLogElm;
    int inputIndex = 0;

    if (elm->elementType() == ElementType::IC) {
        auto *ic = qobject_cast<IC *>(elm);
        currentLogElm = ic->icInput(inputPort->index());
    } else {
        currentLogElm = elm->logic();
        inputIndex = inputPort->index();
    }

    const int connections = inputPort->connections().size();

    if ((connections == 0) && !inputPort->isRequired()) {
        auto *predecessorLogic = (inputPort->defaultValue() == Status::Active) ? &m_globalVCC : &m_globalGND;
        currentLogElm->connectPredecessor(inputIndex, predecessorLogic, 0);
    }

    // TODO: and if there is more than one input? use std::any_of?
    if (connections == 1) {
        if (QNEPort *otherPort = inputPort->connections().constFirst()->otherPort(inputPort)) {
            if (auto *predecessorElement = otherPort->graphicElement()) {
                LogicElement *predOutElm;
                int predOutIndex = 0;

                if (auto *ic = qobject_cast<IC *>(predecessorElement)) {
                    predOutElm = ic->icOutput(otherPort->index());
                } else {
                    predOutElm = predecessorElement->logic();
                    predOutIndex = otherPort->index();
                }

                currentLogElm->connectPredecessor(inputIndex, predOutElm, predOutIndex);
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
