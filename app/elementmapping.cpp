// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementmapping.h"
#include "clock.h"
#include "ic.h"
#include "icmanager.h"
#include "icmapping.h"
#include "icprototype.h"
#include "graphicelement.h"
#include "qneconnection.h"

#include "logicelement/logicand.h"
#include "logicelement/logicdemux.h"
#include "logicelement/logicdflipflop.h"
#include "logicelement/logicdlatch.h"
#include "logicelement/logicjkflipflop.h"
#include "logicelement/logicmux.h"
#include "logicelement/logicnand.h"
#include "logicelement/logicnode.h"
#include "logicelement/logicnor.h"
#include "logicelement/logicnot.h"
#include "logicelement/logicor.h"
#include "logicelement/logicoutput.h"
#include "logicelement/logicsrflipflop.h"
#include "logicelement/logictflipflop.h"
#include "logicelement/logicxnor.h"
#include "logicelement/logicxor.h"

#include <QDebug>

ElementMapping::ElementMapping(const QVector<GraphicElement *> &elms, QString file)
    : currentFile(file)
    , initialized(false)
    , elements(elms)
    , globalGND(false)
    , globalVCC(true)
{
}

ElementMapping::~ElementMapping()
{
    clear();
}

void ElementMapping::clear()
{
    initialized = false;
    globalGND.clearSucessors();
    globalVCC.clearSucessors();
    for (LogicElement *elm : qAsConst(deletableElements)) {
        delete elm;
    }
    deletableElements.clear();
    for (ICMapping *icMap : qAsConst(icMappings)) {
        delete icMap;
    }
    icMappings.clear();
    map.clear();
    inputMap.clear();
    clocks.clear();
    logicElms.clear();
}

QVector<GraphicElement *> ElementMapping::sortGraphicElements(QVector<GraphicElement *> elms)
{
    //! Clazy warning: Use QHash<K, T> instead of QMap<K, T> when K is a pointer
    QHash<GraphicElement *, bool> beingvisited;
    QHash<GraphicElement *, int> priority;
    for (GraphicElement *elm : elms) {
        calculatePriority(elm, beingvisited, priority);
    }
    std::sort(elms.begin(), elms.end(), [priority](GraphicElement *e1, GraphicElement *e2) {
        return priority[e2] < priority[e1];
    });

    return elms;
}

void ElementMapping::insertElement(GraphicElement *elm)
{
    LogicElement *logicElm = buildLogicElement(elm);
    elm->type();
    deletableElements.append(logicElm);
    logicElms.append(logicElm);
    map.insert(elm, logicElm);
    auto *in = dynamic_cast<Input *>(elm);
    if (in) {
        inputMap[in] = logicElm;
    }
}

void ElementMapping::insertIC(IC *ic)
{
    Q_ASSERT(ic);
    Q_ASSERT(!icMappings.contains(ic));
    ICPrototype *proto = ic->getPrototype();
    if (proto) {
        ICMapping *icMap = proto->generateMapping();
        Q_ASSERT(icMap);
        icMap->initialize();
        icMappings.insert(ic, icMap);
        logicElms.append(icMap->logicElms);
    }
}

void ElementMapping::generateMap()
{
    for (GraphicElement *elm : qAsConst(elements)) {
        if (elm->elementType() == ElementType::CLOCK) {
            auto *clk = dynamic_cast<Clock *>(elm);
            Q_ASSERT(clk != nullptr);
            clocks.append(clk);
        }
        if (elm->elementType() == ElementType::IC) {
            IC *ic = dynamic_cast<IC *>(elm);
            insertIC(ic);
        } else {
            insertElement(elm);
        }
    }
}

LogicElement *ElementMapping::buildLogicElement(GraphicElement *elm)
{
    switch (elm->elementType()) {
    case ElementType::SWITCH:
    case ElementType::BUTTON:
    case ElementType::CLOCK:
        return new LogicInput();
    case ElementType::LED:
    case ElementType::BUZZER:
    case ElementType::DISPLAY:
    case ElementType::DISPLAY14:
        return new LogicOutput(elm->inputSize());
    case ElementType::NODE:
        return new LogicNode();
    case ElementType::VCC:
        return new LogicInput(true);
    case ElementType::GND:
        return new LogicInput(false);
    case ElementType::AND:
        return new LogicAnd(elm->inputSize());
    case ElementType::OR:
        return new LogicOr(elm->inputSize());
    case ElementType::NAND:
        return new LogicNand(elm->inputSize());
    case ElementType::NOR:
        return new LogicNor(elm->inputSize());
    case ElementType::XOR:
        return new LogicXor(elm->inputSize());
    case ElementType::XNOR:
        return new LogicXnor(elm->inputSize());
    case ElementType::NOT:
        return new LogicNot();
    case ElementType::JKFLIPFLOP:
        return new LogicJKFlipFlop();
    case ElementType::SRFLIPFLOP:
        return new LogicSRFlipFlop();
    case ElementType::TFLIPFLOP:
        return new LogicTFlipFlop();
    case ElementType::DFLIPFLOP:
        return new LogicDFlipFlop();
    case ElementType::DLATCH:
        return new LogicDLatch();
    case ElementType::MUX:
        return new LogicMux();
    case ElementType::DEMUX:
        return new LogicDemux();
        //      case ElementType::TLATCH:
    case ElementType::JKLATCH:
        //! TODO: TLATCH not yet implemented.
        return new LogicDLatch();

    default:
        throw std::runtime_error("Not implemented yet: " + elm->objectName().toStdString());
    }
}

void ElementMapping::initialize()
{
    clear();
    generateMap();
    connectElements();
    initialized = true;
}

void ElementMapping::sort()
{
    sortLogicElements();
    validateElements();
}

// TODO: This function can easily cause crashes when using the Undo command to delete elements
void ElementMapping::update()
{
    //  bool resetSimulationController = false;
    if (canRun()) {
        for (Clock *clk : qAsConst(clocks)) {
            if (!clk) {
                continue;
            };
            if (Clock::reset) {
                clk->resetClock();
            } else {
                clk->updateClock();
            }
        }
        Clock::reset = false;
        for (auto iter = inputMap.begin(); iter != inputMap.end(); ++iter) {
            if (!iter.key()) {
                continue;
            }
            if (!iter.value()) {
                continue;
            }
            iter.value()->setOutputValue(iter.key()->getOn());
        }
        for (LogicElement *elm : qAsConst(logicElms)) {
            elm->updateLogic();
        }
    }
    //  return resetSimulationController;
}

ICMapping *ElementMapping::getICMapping(IC *ic) const
{
    Q_ASSERT(ic);
    return icMappings[ic];
}

LogicElement *ElementMapping::getLogicElement(GraphicElement *elm) const
{
    Q_ASSERT(elm);
    return map[elm];
}

bool ElementMapping::canRun() const
{
    return initialized;
}

bool ElementMapping::canInitialize() const
{
    for (GraphicElement *elm : elements) {
        if (elm->elementType() == ElementType::IC) {
            IC *ic = dynamic_cast<IC *>(elm);
            ICPrototype *prototype = ICManager::instance()->getPrototype(ic->getFile());
            if (!ic || !prototype) {
                return false;
            }
        }
    }
    return true;
}

void ElementMapping::applyConnection(GraphicElement *elm, QNEPort *in)
{
    LogicElement *currentLogElm;
    int inputIndex = 0;
    if (elm->elementType() == ElementType::IC) {
        IC *ic = dynamic_cast<IC *>(elm);
        Q_ASSERT(ic);
        currentLogElm = icMappings[ic]->getInput(in->index());
    } else {
        currentLogElm = map[elm];
        inputIndex = in->index();
    }
    Q_ASSERT(currentLogElm);
    int connections = in->connections().size();
    bool connection_required = in->isRequired();
    if (connections == 1) {
        QNEPort *other_out = in->connections().first()->otherPort(in);
        if (other_out) {
            GraphicElement *predecessor = other_out->graphicElement();
            if (predecessor) {
                int predOutIndex = 0;
                LogicElement *predOutElm;
                if (predecessor->elementType() == ElementType::IC) {
                    IC *ic = dynamic_cast<IC *>(predecessor);
                    Q_ASSERT(ic);
                    Q_ASSERT(icMappings.contains(ic));
                    predOutElm = icMappings[ic]->getOutput(other_out->index());
                } else {
                    predOutElm = map[predecessor];
                    predOutIndex = other_out->index();
                }
                currentLogElm->connectPredecessor(inputIndex, predOutElm, predOutIndex);
            }
        }
    } else if ((connections == 0) && (!connection_required)) {
        LogicElement *pred = in->defaultValue() ? &globalVCC : &globalGND;
        currentLogElm->connectPredecessor(inputIndex, pred, 0);
    }
}

void ElementMapping::connectElements()
{
    for (GraphicElement *elm : qAsConst(elements)) {
        const auto elm_inputs = elm->inputs();
        for (QNEPort *in : elm_inputs) {
            applyConnection(elm, in);
        }
    }
}

void ElementMapping::validateElements()
{
    for (LogicElement *elm : qAsConst(logicElms)) {
        elm->validate();
    }
}

void ElementMapping::sortLogicElements()
{
    for (LogicElement *elm : qAsConst(logicElms)) {
        elm->calculatePriority();
    }
    std::sort(logicElms.begin(), logicElms.end(), [](LogicElement *e1, LogicElement *e2) {
        return *e2 < *e1;
    });
}

int ElementMapping::calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingvisited, QHash<GraphicElement *, int> &priority)
{
    if (!elm) {
        return 0;
    }
    if (beingvisited.contains(elm) && (beingvisited[elm])) {
        return 0;
    }
    if (priority.contains(elm)) {
        return priority[elm];
    }
    beingvisited[elm] = true;
    int max = 0;
    auto const elm_outputs = elm->outputs();
    for (QNEPort *port : elm_outputs) {
        for (QNEConnection *conn : port->connections()) {
            QNEPort *sucessor = conn->otherPort(port);
            if (sucessor) {
                max = qMax(calculatePriority(sucessor->graphicElement(), beingvisited, priority), max);
            }
        }
    }
    int p = max + 1;
    priority[elm] = p;
    beingvisited[elm] = false;
    return p;
}
