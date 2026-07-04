// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICSimulation: builds and runs an IC's internal simulation graph.
 */

#pragma once

class IC;

/**
 * \class ICSimulation
 * \brief Builds and drives an IC's internal simulation graph.
 *
 * \details Extracted from IC so direct simulation is a collaborator rather than a
 * cluster of methods on the element itself, mirroring the GraphicElement /
 * GraphicElementSerializer split. A friend of IC, it reaches the element's internal
 * simulation state directly. All methods are static and take the IC explicitly; IC
 * keeps all state, so this is a pure logic extraction, not a new owner-back-pointer
 * collaborator.
 */
class ICSimulation
{
public:
    /// Builds the internal simulation graph (connection graph + topological sort) for \a ic.
    static void initialize(IC &ic);

    /// Simulates \a ic's internal circuit and propagates results to its boundary.
    static void update(IC &ic);

    /// Re-propagates committed sequential state through \a ic's internal combinational
    /// logic and refreshes its output boundary, without re-clocking internal
    /// flip-flops/latches.
    static void resettle(IC &ic);

private:
    /// Copies \a ic's external input values onto its boundary input nodes.
    static void pushInputsToBoundary(IC &ic);
    /// Copies \a ic's boundary output nodes' values onto its external outputs.
    static void pullOutputsFromBoundary(IC &ic);
};
