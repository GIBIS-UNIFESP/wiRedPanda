// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICSimulation: builds and runs an IC's internal direct-simulation graph.
 */

#pragma once

class IC;

/**
 * \class ICSimulation
 * \brief Builds the internal connection graph + topological order for an IC and runs its
 * per-tick direct simulation.
 *
 * \details Extracted from IC (a friend of it) so the internal-simulation concern is separate
 * from loading and rendering. Operates on the IC's internal element/connection/port vectors
 * and its derived sort/feedback state through that friendship. IC keeps the public
 * initializeSimulation() and the virtual updateLogic() as thin entry points that delegate here.
 */
class ICSimulation
{
public:
    /// Builds the internal connection graph, wires wireless Tx/Rx, recurses into nested ICs,
    /// records boundary input elements, and topologically sorts (detecting feedback).
    static void initialize(IC &ic);

    /// Runs one simulation tick: pushes external inputs to boundary nodes, evaluates the
    /// internal elements in topological order (or iteratively when feedback is present), and
    /// pulls results back to the IC's external outputs.
    static void update(IC &ic);
};
