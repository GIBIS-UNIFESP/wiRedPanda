// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementmapping.h"

#include "clock.h"
#include "common.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmanager.h"
#include "icmapping.h"
#include "icprototype.h"
#include "input.h"
#include "logicelement.h"
#include "qneconnection.h"
#include "qneport.h"

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
    m_elementMap.clear();
    m_inputMap.clear();
    m_clocks.clear();
    m_logicElms.clear();
}

QVector<GraphicElement *> ElementMapping::sortGraphicElements(QVector<GraphicElement *> elms)
{
    QHash<GraphicElement *, bool> beingvisited;
    QHash<GraphicElement *, int> priority;
    for (auto *elm : elms) {
        calculatePriority(elm, beingvisited, priority);
    }
    std::sort(elms.begin(), elms.end(), [priority](const auto &e1, const auto &e2) {
        return priority[e1] > priority[e2];
    });

    return elms;
}

void ElementMapping::insertElement(GraphicElement *elm)
{
    LogicElement *logicElm = buildLogicElement(elm);
    m_deletableElements.append(logicElm);
    m_logicElms.append(logicElm);
    m_elementMap.insert(elm, logicElm);
    auto *in = dynamic_cast<Input *>(elm);
    if (in) {
        m_inputMap[in] = logicElm;
    }
}

void ElementMapping::insertIC(IC *ic)
{
    ICPrototype *proto = ic->prototype();
    if (proto) {
        ICMapping *icMap = proto->generateMapping();
        icMap->initialize();
        m_icMappings.insert(ic, icMap);
        m_logicElms.append(icMap->m_logicElms);
    }
}

void ElementMapping::generateMap()
{
    for (auto *elm : qAsConst(m_elements)) {
        if (elm->elementType() == ElementType::Clock) {
            auto *clk = dynamic_cast<Clock *>(elm);
            m_clocks.append(clk);
        }
        if (elm->elementType() == ElementType::IC) {
            auto *ic = dynamic_cast<IC *>(elm);
            insertIC(ic);
        } else {
            insertElement(elm);
        }
    }
}

LogicElement *ElementMapping::buildLogicElement(GraphicElement *elm)
{
    switch (elm->elementType()) {
    case ElementType::InputSwitch:
    case ElementType::InputButton:
    case ElementType::Clock:
    case ElementType::InputRotary: return new LogicInput(false, elm->outputSize());
    case ElementType::Led:
    case ElementType::Buzzer:
    case ElementType::Display:
    case ElementType::Display14:   return new LogicOutput(elm->inputSize());
    case ElementType::Node:        return new LogicNode();
    case ElementType::InputVcc:    return new LogicInput(true);
    case ElementType::InputGnd:    return new LogicInput(false);
    case ElementType::And:         return new LogicAnd(elm->inputSize());
    case ElementType::Or:          return new LogicOr(elm->inputSize());
    case ElementType::Nand:        return new LogicNand(elm->inputSize());
    case ElementType::Nor:         return new LogicNor(elm->inputSize());
    case ElementType::Xor:         return new LogicXor(elm->inputSize());
    case ElementType::Xnor:        return new LogicXnor(elm->inputSize());
    case ElementType::Not:         return new LogicNot();
    case ElementType::JKFlipFlop:  return new LogicJKFlipFlop();
    case ElementType::SRFlipFlop:  return new LogicSRFlipFlop();
    case ElementType::TFlipFlop:   return new LogicTFlipFlop();
    case ElementType::DFlipFlop:   return new LogicDFlipFlop();
    case ElementType::DLatch:      return new LogicDLatch();
    case ElementType::Mux:         return new LogicMux();
    case ElementType::Demux:       return new LogicDemux();
 // case ElementType::TLATCH:
    case ElementType::JKLatch:     return new LogicDLatch();
    case ElementType::Text:
    case ElementType::Line:        return new LogicNone();
    default:                       throw Pandaception(tr("Not implemented yet: ") + elm->objectName());
    }
}

void ElementMapping::initialize()
{
    qCDebug(three) << "Clear.";
    clear();
    qCDebug(three) << "Generate Map.";
    generateMap();
    qCDebug(three) << "Connect.";
    connectElements();
    m_initialized = true;
}

void ElementMapping::sort()
{
    sortLogicElements();
    validateElements();
}

// TODO: This function can easily cause crashes when using the Undo command to delete elements
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

    for (auto iter = m_inputMap.begin(); iter != m_inputMap.end(); ++iter) {
        if (!iter.key() || !iter.value()) {
            continue;
        }

        for (int port = 0; port < iter.key()->outputSize(); ++port) {
            iter.value()->setOutputValue(port, iter.key()->on(port));
        }
    }

    for (auto *elm : qAsConst(m_logicElms)) {
        elm->updateLogic();
    }
}

ICMapping *ElementMapping::icMapping(IC *ic) const
{
    return m_icMappings.value(ic);
}

LogicElement *ElementMapping::logicElement(GraphicElement *elm) const
{
    return m_elementMap.value(elm);
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
        auto *prototype = ICManager::prototype(ic->file());
        return ic && prototype;
    });
}

void ElementMapping::applyConnection(GraphicElement *elm, QNEPort *in)
{
    LogicElement *currentLogElm;
    int inputIndex = 0;
    if (elm->elementType() == ElementType::IC) {
        auto *ic = dynamic_cast<IC *>(elm);
        currentLogElm = m_icMappings.value(ic)->input(in->index());
    } else {
        currentLogElm = m_elementMap.value(elm);
        inputIndex = in->index();
    }
    int connections = in->connections().size();
    bool connectionRequired = in->isRequired();
    // TODO: and if there is more than one input? use std::any_of?
    if (connections == 1) {
        QNEPort *otherOut = in->connections().first()->otherPort(in);
        if (otherOut) {
            GraphicElement *predecessor = otherOut->graphicElement();
            if (predecessor) {
                int predOutIndex = 0;
                LogicElement *predOutElm;
                if (predecessor->elementType() == ElementType::IC) {
                    IC *ic = dynamic_cast<IC *>(predecessor);
                    predOutElm = m_icMappings.value(ic)->output(otherOut->index());
                } else {
                    predOutElm = m_elementMap.value(predecessor);
                    predOutIndex = otherOut->index();
                }
                currentLogElm->connectPredecessor(inputIndex, predOutElm, predOutIndex);
            }
        }
    } else if ((connections == 0) && (!connectionRequired)) {
        LogicElement *pred = (in->defaultValue()) ? &m_globalVCC : &m_globalGND;
        currentLogElm->connectPredecessor(inputIndex, pred, 0);
    }
}

void ElementMapping::connectElements()
{
    for (auto *elm : qAsConst(m_elements)) {
        const auto elmInputs = elm->inputs();
        for (auto *in : elmInputs) {
            applyConnection(elm, in);
        }
    }
}

void ElementMapping::validateElements()
{
    for (auto *elm : qAsConst(m_logicElms)) {
        elm->validate();
    }
}

void ElementMapping::sortLogicElements()
{
    for (auto *elm : qAsConst(m_logicElms)) {
        elm->calculatePriority();
    }
    std::sort(m_logicElms.begin(), m_logicElms.end(), [](const auto &e1, const auto &e2) {
        return e1 > e2;
    });
}

int ElementMapping::calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingVisited, QHash<GraphicElement *, int> &priority)
{
    if (!elm) {
        return 0;
    }
    if (beingVisited.contains(elm) && (beingVisited.value(elm))) {
        return 0;
    }
    if (priority.contains(elm)) {
        return priority.value(elm);
    }
    beingVisited[elm] = true;
    int max = 0;
    const auto elmOutputs = elm->outputs();
    for (auto *port : elmOutputs) {
        for (auto *conn : port->connections()) {
            auto *successor = conn->otherPort(port);
            if (successor) {
                max = qMax(calculatePriority(successor->graphicElement(), beingVisited, priority), max);
            }
        }
    }
    int p = max + 1;
    priority[elm] = p;
    beingVisited[elm] = false;
    return p;
}
