// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementmapping.h"

#include "clock.h"
#include "graphicelement.h"
#include "graphicelementinput.h"
#include "ic.h"
#include "icmanager.h"
#include "icmapping.h"
#include "icprototype.h"
#include "qneconnection.h"

#include "logicand.h"
#include "logicdemux.h"
#include "logicdflipflop.h"
#include "logicdlatch.h"
#include "logicjkflipflop.h"
#include "logicmux.h"
#include "logicnand.h"
#include "logicnode.h"
#include "logicnone.h"
#include "logicnor.h"
#include "logicnot.h"
#include "logicor.h"
#include "logicoutput.h"
#include "logicsrflipflop.h"
#include "logictflipflop.h"
#include "logicxnor.h"
#include "logicxor.h"

ElementMapping::ElementMapping(const QVector<GraphicElement *> &elms)
    : m_elements(elms)
{
}

ElementMapping::~ElementMapping()
{
    clear();
}

void ElementMapping::clear()
{
    m_initialized = false;
    m_globalGND.clearSucessors();
    m_globalVCC.clearSucessors();
    qDeleteAll(m_deletableElements);
    m_deletableElements.clear();
    qDeleteAll(m_icMappings);
    m_icMappings.clear();
    m_inputs.clear();
    m_clocks.clear();
    m_logicElms.clear();
}

QVector<GraphicElement *> ElementMapping::sortGraphicElements(QVector<GraphicElement *> elms)
{
    QHash<GraphicElement *, bool> beingVisited;
    QHash<GraphicElement *, int> priorities;

    for (auto *elm : elms) {
        calculatePriority(elm, beingVisited, priorities);
    }

    std::sort(elms.begin(), elms.end(), [priorities](const auto &e1, const auto &e2) {
        return priorities[e2] < priorities[e1];
    });

    return elms;
}

void ElementMapping::insertElement(GraphicElement *elm)
{
    LogicElement *logicElm = buildLogicElement(elm);
    elm->setLogic(logicElm);
    m_deletableElements.append(logicElm);
    m_logicElms.append(logicElm);
}

void ElementMapping::insertIC(IC *ic)
{
    if (!ic) {
        return;
    }

    if (ICPrototype *prototype = ic->prototype()) {
        ICMapping *icMap = prototype->generateMapping();
        icMap->initialize();
        m_icMappings.insert(ic, icMap);
        m_logicElms.append(icMap->m_logicElms);
    }
}

void ElementMapping::generateMap()
{
    for (auto *elm : qAsConst(m_elements)) {
        if (elm->elementType() == ElementType::IC) {
            insertIC(dynamic_cast<IC *>(elm));
            continue;
        }

        if (elm->elementType() == ElementType::Clock) {
            m_clocks.append(dynamic_cast<Clock *>(elm));
        }

        if (elm->elementGroup() == ElementGroup::Input) {
            m_inputs.append(dynamic_cast<GraphicElementInput *>(elm));
        }

        insertElement(elm);
    }
}

LogicElement *ElementMapping::buildLogicElement(GraphicElement *elm)
{
    switch (elm->elementType()) {
    case ElementType::Clock:
    case ElementType::InputButton:
    case ElementType::InputRotary:
    case ElementType::InputSwitch: return new LogicInput(false, elm->outputSize());

    case ElementType::Buzzer:
    case ElementType::Display14:
    case ElementType::Display:
    case ElementType::Led:         return new LogicOutput(elm->inputSize());

    case ElementType::And:         return new LogicAnd(elm->inputSize());
    case ElementType::DFlipFlop:   return new LogicDFlipFlop();
    case ElementType::Demux:       return new LogicDemux();
    case ElementType::InputGnd:    return new LogicInput(false);
    case ElementType::InputVcc:    return new LogicInput(true);
    case ElementType::JKFlipFlop:  return new LogicJKFlipFlop();
    case ElementType::Mux:         return new LogicMux();
    case ElementType::Nand:        return new LogicNand(elm->inputSize());
    case ElementType::Node:        return new LogicNode();
    case ElementType::Nor:         return new LogicNor(elm->inputSize());
    case ElementType::Not:         return new LogicNot();
    case ElementType::Or:          return new LogicOr(elm->inputSize());
    case ElementType::SRFlipFlop:  return new LogicSRFlipFlop();
    case ElementType::TFlipFlop:   return new LogicTFlipFlop();
    case ElementType::Xnor:        return new LogicXnor(elm->inputSize());
    case ElementType::Xor:         return new LogicXor(elm->inputSize());

    case ElementType::DLatch:      return new LogicDLatch();

    case ElementType::Line:
    case ElementType::Text:        return new LogicNone();

    default:                       throw Pandaception(tr("Not implemented yet: ") + elm->objectName());
    }
}

void ElementMapping::initialize()
{
    qCDebug(three) << tr("Clear.");
    clear();
    qCDebug(three) << tr("Generate Map.");
    generateMap();
    qCDebug(three) << tr("Connect.");
    connectElements();
    m_initialized = true;
}

void ElementMapping::sort()
{
    sortLogicElements();
    validateElements();
}

// FIXME: This function can easily cause crashes when using the Undo command to delete elements
void ElementMapping::update()
{
    if (!canRun()) {
        return;
    }

    for (auto *clk : qAsConst(m_clocks)) {
        if (!clk) {
            continue;
        };

        Clock::reset ? clk->resetClock() : clk->updateClock();
    }

    Clock::reset = false;

    for (auto *input : qAsConst(m_inputs)) {
        input->updatePortsOutputs();
    }

    for (auto *logic : qAsConst(m_logicElms)) {
        logic->updateLogic();
    }
}

ICMapping *ElementMapping::icMapping(IC *ic) const
{
    return m_icMappings.value(ic);
}

bool ElementMapping::canRun() const
{
    return m_initialized;
}

bool ElementMapping::canInitialize() const
{
    return std::all_of(m_elements.cbegin(), m_elements.cend(), [](auto &elm) {
        if (elm->elementType() != ElementType::IC) {
            return true;
        }

        auto *ic = dynamic_cast<IC *>(elm);
        auto *prototype = (ic) ? ICManager::prototype(ic->file()) : nullptr;
        return (ic && prototype);
    });
}

void ElementMapping::applyConnection(GraphicElement *elm, QNEPort *portIn)
{
    LogicElement *currentLogElm;
    int inputIndex = 0;

    if (elm->elementType() == ElementType::IC) {
        auto *ic = dynamic_cast<IC *>(elm);
        currentLogElm = m_icMappings.value(ic)->input(portIn->index());
    } else {
        currentLogElm = elm->logic();
        inputIndex = portIn->index();
    }

    int connections = portIn->connections().size();
    bool connectionRequired = portIn->isRequired();

    // TODO: and if there is more than one input? use std::any_of?
    if (connections == 1) {
        if (QNEPort *otherOut = portIn->connections().first()->otherPort(portIn)) {
            if (GraphicElement *predecessor = otherOut->graphicElement()) {
                LogicElement *predOutElm;
                int predOutIndex = 0;

                if (auto *ic = dynamic_cast<IC *>(predecessor)) {
                    predOutElm = m_icMappings.value(ic)->output(otherOut->index());
                } else {
                    predOutElm = predecessor->logic();
                    predOutIndex = otherOut->index();
                }

                currentLogElm->connectPredecessor(inputIndex, predOutElm, predOutIndex);
            }
        }
    }

    if ((connections == 0) && !connectionRequired) {
        LogicElement *pred = (portIn->defaultValue() == Status::Active) ? &m_globalVCC : &m_globalGND;
        currentLogElm->connectPredecessor(inputIndex, pred, 0);
    }
}

void ElementMapping::connectElements()
{
    for (auto *elm : qAsConst(m_elements)) {
        const auto ports = elm->inputs();

        for (auto *portIn : ports) {
            applyConnection(elm, portIn);
        }
    }
}

void ElementMapping::validateElements()
{
    for (auto *logic : qAsConst(m_logicElms)) {
        logic->validate();
    }
}

void ElementMapping::sortLogicElements()
{
    for (auto *logic : qAsConst(m_logicElms)) {
        logic->calculatePriority();
    }

    std::sort(m_logicElms.begin(), m_logicElms.end(), [](const auto &logic1, const auto &logic2) {
        return *logic2 < *logic1;
    });
}

int ElementMapping::calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingVisited, QHash<GraphicElement *, int> &priorities)
{
    if (!elm) {
        return 0;
    }

    if (beingVisited.contains(elm) && (beingVisited.value(elm))) {
        return 0;
    }

    if (priorities.contains(elm)) {
        return priorities.value(elm);
    }

    beingVisited[elm] = true;
    int max = 0;
    const auto elmOutputs = elm->outputs();

    for (auto *port : elmOutputs) {
        for (auto *conn : port->connections()) {
            if (auto *successor = conn->otherPort(port)) {
                max = qMax(calculatePriority(successor->graphicElement(), beingVisited, priorities), max);
            }
        }
    }

    int priority = max + 1;
    priorities[elm] = priority;
    beingVisited[elm] = false;
    return priority;
}
