// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICSimulation.h"

#include <algorithm>

#include "App/Element/IC.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Port.h"

void ICSimulation::initialize(IC &ic)
{
    ic.m_sortedInternalElements.clear();
    ic.m_boundaryInputElements.clear();
    ic.m_internalHasFeedback = false;
    ic.m_internalSuccessors.clear();
    ic.m_internalPriorities.clear();

    if (ic.m_internalElements.isEmpty()) {
        return;
    }

    // Initialize simulation vectors on all internal elements
    for (auto *elm : std::as_const(ic.m_internalElements)) {
        elm->initSimulationVectors(elm->inputSize(), elm->outputSize());
    }

    // Build connection graph and wire wireless Tx→Rx
    Simulation::buildConnectionGraph(ic.m_internalElements);
    Simulation::connectWirelessElements(ic.m_internalElements);

    // Initialize nested ICs recursively
    for (auto *elm : std::as_const(ic.m_internalElements)) {
        if (elm->elementType() == ElementType::IC) {
            ICSimulation::initialize(*static_cast<IC *>(elm));
        }
    }

    // Record boundary input elements (driven externally, should not run updateLogic)
    for (auto *port : std::as_const(ic.m_internalInputs)) {
        if (auto *elm = port->graphicElement()) {
            ic.m_boundaryInputElements.insert(elm);
        }
    }

    // Topological sort with feedback detection using shared helpers
    const auto txMap = Simulation::buildTxMap(ic.m_internalElements);
    const auto successors = Simulation::buildSuccessorGraph(ic.m_internalElements, txMap);
    const auto result = Simulation::topologicalSort(ic.m_internalElements, successors);
    ic.m_internalHasFeedback = !result.feedbackNodes.isEmpty();

    // Persist the internal graph for the event-driven settle (eventSettle) in updateLogic().
    ic.m_internalSuccessors = successors;
    ic.m_internalPriorities.clear();
    for (auto *elm : std::as_const(ic.m_internalElements)) {
        ic.m_internalPriorities[elm] = result.priorities.value(elm, -1);
    }

    // Remove boundary input elements so the hot loop in updateLogic()
    // doesn't need a per-element QSet lookup on every simulation tick.
    ic.m_sortedInternalElements = result.sorted;
    ic.m_sortedInternalElements.erase(
        std::remove_if(ic.m_sortedInternalElements.begin(), ic.m_sortedInternalElements.end(),
            [&ic](auto *elm) { return ic.m_boundaryInputElements.contains(elm); }),
        ic.m_sortedInternalElements.end());
}

void ICSimulation::pushInputsToBoundary(IC &ic)
{
    for (int i = 0; i < ic.inputSize() && i < ic.m_internalInputs.size(); ++i) {
        auto *boundaryElement = ic.m_internalInputs.at(i)->graphicElement();
        if (boundaryElement) {
            boundaryElement->setOutputValue(0, ic.simInputs().at(i));
        }
    }
}

void ICSimulation::pullOutputsFromBoundary(IC &ic)
{
    for (int i = 0; i < ic.outputSize() && i < ic.m_internalOutputs.size(); ++i) {
        auto *boundaryElement = ic.m_internalOutputs.at(i)->graphicElement();
        if (boundaryElement) {
            ic.setOutputValue(i, boundaryElement->outputValue(0));
        }
    }
}

void ICSimulation::update(IC &ic)
{
    if (ic.m_sortedInternalElements.isEmpty()) {
        return;
    }

    // Permissive mode so ICs can propagate Unknown through their internal elements.
    if (!ic.simUpdateInputsAllowUnknown()) {
        return;
    }

    pushInputsToBoundary(ic);

    // Settle the internal elements with the shared event-driven engine (one code path for
    // combinational and feedback ICs). Boundary input nodes are already excluded from the seed.
    // Internal sequential elements were already bracketed with beginDeferredCommit() by the
    // top-level Simulation::update() (collectSequentialElements() recurses into ICs), so this
    // call honors non-blocking semantics the same way the top-level processEvents() does.
    Simulation::eventSettle(ic.m_sortedInternalElements, ic.m_internalSuccessors, ic.m_internalPriorities);

    pullOutputsFromBoundary(ic);
}

void ICSimulation::resettle(IC &ic)
{
    if (ic.m_sortedInternalElements.isEmpty()) {
        return;
    }

    if (!ic.simUpdateInputsAllowUnknown()) {
        return;
    }

    pushInputsToBoundary(ic);

    // Re-propagate just-committed internal sequential state through the IC's
    // combinational logic, WITHOUT re-running the sequential elements (that
    // would corrupt their edge state / re-clock them). Memory-group internals
    // keep the value they committed this tick; everything else recomputes.
    // Most cycles run through a sequential element (register/counter cells), so
    // removing them leaves an acyclic graph that settles in one topological
    // sweep; only genuine combinational feedback (gate-built latches) needs more.
    // Iterate to a fixed point, bounded, instead of a fixed pass count.
    const int maxPasses = ic.m_internalHasFeedback ? Simulation::kMaxSettleIterations : 1;
    for (int pass = 0; pass < maxPasses; ++pass) {
        bool changed = false;
        for (auto *element : std::as_const(ic.m_sortedInternalElements)) {
            if (element && element->elementGroup() != ElementGroup::Memory) {
                element->clearOutputChanged();
                element->resettleCombinational();
                changed = changed || element->outputChanged();
            }
        }
        if (!changed) {
            break;
        }
    }

    pullOutputsFromBoundary(ic);
}
