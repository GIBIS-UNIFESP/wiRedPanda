// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WirelessNode: a node's wireless routing mode, read as the enum it is.
 */

#pragma once

#include "App/Core/Enums.h"

class Element;

/// A node's wireless routing mode (ParamId::Wireless), which its parameter holds as an int: a transmitter broadcasts
/// its input to the receivers sharing its label.
namespace WirelessNode {

/// Returns the wireless routing mode of \a element; WirelessMode::None for a type without one, which routes nothing.
WirelessMode mode(const Element &element);

/// Sets the wireless routing mode of \a element to \a mode, as its spec takes it (a mode out of range is None).
void setMode(Element &element, WirelessMode mode);

} // namespace WirelessNode
