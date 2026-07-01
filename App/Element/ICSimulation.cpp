// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICSimulation.h"

#include "App/Element/IC.h"
#include "App/Simulation/Simulation.h"

void ICSimulation::initialize(IC &ic)
{
    if (ic.m_internalElements.isEmpty()) {
        return;
    }

    // The IC contributes only structure to the top-level flat netlist: it wires its internal
    // primitives (here) and exposes them via internalElements()/internalInputs()/
    // internalOutputs(); Simulation::initialize() splices the boundary and simulates the
    // primitives directly. The IC itself no longer settles (no updateLogic).

    // Initialize simulation vectors on all internal elements
    for (auto *elm : std::as_const(ic.m_internalElements)) {
        elm->initSimulationVectors(elm->inputSize(), elm->outputSize());
    }

    // Build connection graph and wire wireless Tx→Rx (per-IC scope, so same-label wireless
    // in different IC instances stays isolated)
    Simulation::buildConnectionGraph(ic.m_internalElements);
    Simulation::connectWirelessElements(ic.m_internalElements);

    // Initialize nested ICs recursively
    for (auto *elm : std::as_const(ic.m_internalElements)) {
        if (elm->elementType() == ElementType::IC) {
            ICSimulation::initialize(*static_cast<IC *>(elm));
        }
    }
}
