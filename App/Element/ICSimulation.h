// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICSimulation: wires an IC's internal primitives for the top-level flat netlist.
 */

#pragma once

class IC;

/**
 * \class ICSimulation
 * \brief Wires an IC's internal primitives so Simulation can splice them into its flat netlist.
 *
 * \details Extracted from IC so direct simulation is a collaborator rather than a
 * cluster of methods on the element itself, mirroring the GraphicElement /
 * GraphicElementSerializer split. A friend of IC, it reaches the element's internal
 * simulation state directly. The IC itself no longer settles: Simulation::initialize()
 * flattens every IC's primitives (recursively, through nesting) into one flat netlist,
 * so each internal gate's propagation delay applies through the IC boundary.
 */
class ICSimulation
{
public:
    /// Wires \a ic's internal connections (and recurses into nested ICs) so
    /// Simulation::initialize() can splice the boundary and simulate the primitives
    /// directly. Does not build a settle graph — the IC no longer simulates itself.
    static void initialize(IC &ic);
};
