// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Pure, Widgets-free wire-connection validity rules -- extracted from
 * ConnectionManager so portable consumers (a future Quick canvas, tests) don't need to link
 * against the App/Scene sources.
 */

#pragma once

#include <QString>

class InputPort;
class OutputPort;

/// Returns \c true if a physical wire from \a startPort to \a endPort is allowed. Equivalent
/// to connectionRejectionReason(startPort, endPort).isEmpty().
bool isConnectionAllowed(OutputPort *startPort, InputPort *endPort);

/// Returns a user-facing explanation of why a wire from \a startPort to \a endPort would be
/// rejected, or an empty string if it's allowed -- shown as status-bar feedback instead of the
/// wire silently vanishing. Rejects: a null port, a self-loop (same owning element), an
/// already-connected pair, wiring into an Rx wireless node's input (the simulation would
/// silently ignore a physical wire there in favour of the wireless signal), and wiring out of
/// a Tx wireless node's output (a dead-end transmitter -- no physical wire should bypass the
/// wireless channel).
QString connectionRejectionReason(OutputPort *startPort, InputPort *endPort);
