// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief InputLevel: what the inputs the user operates (the switch, the button, the rotary, the clock) share.
 */

#pragma once

#include "App/Element/Parameters.h"

class Element;

/// The parameters of an input the user operates: its lock (lockSpec()), which every one has, and the level a switch, a
/// button or a clock drives on its one output (levelSpec(), onLevel(), setOnLevel()); a rotary's selection is its own.
namespace InputLevel {

/// Returns the spec of the level (ParamId::On), kept in a file under \a key, or transient with none: a new value
/// replaces the picture, whose slot the level picks, and drives the output.
ParamSpec levelSpec(const char *key);

/// Returns the spec of the lock (ParamId::Locked), which keeps the user from operating the input.
ParamSpec lockSpec();

/// Whether output \a port of \a element is on: its level (ParamId::On), on its one output.
bool onLevel(const Element &element, int port);

/// Sets the level of \a element (ParamId::On) to \a level, on its one output.
void setOnLevel(Element &element, bool level, int port);

/// The slot of the picture \a element shows: 0 off, 1 on.
int levelSlot(const Element &element);

} // namespace InputLevel
