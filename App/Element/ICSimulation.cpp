// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICSimulation.h"

#include <algorithm>

#include "App/Core/Enums.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEPort.h"
#include "App/Simulation/Simulation.h"

void ICSimulation::initialize(IC &ic)
{
    ic.m_sortedInternalElements.clear();
    ic.m_boundaryInputElements.clear();
    ic.m_internalHasFeedback = false;

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

    // Remove boundary input elements so the hot loop in update()
    // doesn't need a per-element QSet lookup on every simulation tick.
    ic.m_sortedInternalElements = result.sorted;
    ic.m_sortedInternalElements.erase(
        std::remove_if(ic.m_sortedInternalElements.begin(), ic.m_sortedInternalElements.end(),
            [&ic](auto *elm) { return ic.m_boundaryInputElements.contains(elm); }),
        ic.m_sortedInternalElements.end());
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

    // Push external input values to boundary input nodes' GraphicElement outputs.
    // Boundary input nodes are Nodes whose input port is in m_internalInputs.
    // We set their output value directly and skip them in the update loop.
    for (int i = 0; i < ic.inputSize() && i < ic.m_internalInputs.size(); ++i) {
        auto *boundaryElement = ic.m_internalInputs.at(i)->graphicElement();
        if (boundaryElement) {
            boundaryElement->setOutputValue(0, ic.simInputs().at(i));
        }
    }

    // Run internal elements in topological order.
    // Boundary input nodes are already excluded from m_sortedInternalElements.
    if (ic.m_internalHasFeedback) {
        Simulation::iterativeSettle(ic.m_sortedInternalElements);
    } else {
        for (auto *element : std::as_const(ic.m_sortedInternalElements)) {
            if (element) {
                element->updateLogic();
            }
        }
    }

    // Pull output values from boundary output nodes
    for (int i = 0; i < ic.outputSize() && i < ic.m_internalOutputs.size(); ++i) {
        auto *boundaryElement = ic.m_internalOutputs.at(i)->graphicElement();
        if (boundaryElement) {
            ic.setOutputValue(i, boundaryElement->outputValue(0));
        }
    }
}
