// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief KernelLogic: what the simulation computes for each kernel an element type names.
 */

#pragma once

#include "App/Element/ElementTypeDescriptor.h"

class Element;

/**
 * \brief The logic of each kernel (KernelKind), which Element::updateLogic() runs through its type's descriptor.
 *
 * \details A type names its kernel once, in its descriptor; the logic of a kind sets an element's outputs from its
 * inputs. The sources (Constant, Input, Clock) drive their outputs themselves (a constant by its ports' defaults, an
 * input by its source level, a clock by its phase), and None and Instance are not computed here, so those kinds have
 * no logic. Plan 6's core replaces this logic kind by kind.
 */
namespace KernelLogic {

/// Sets \a element's outputs from its inputs.
using Logic = void (*)(Element &element);

/// Returns the logic of \a kind, or nullptr for a kind that has none: a source, None or Instance, and, until their
/// families' subclasses go (P3.23), the kinds whose subclasses still compute them.
Logic of(KernelKind kind);

/// Returns whether \a kind is a source: its elements drive their outputs themselves, not through logic.
bool isSource(KernelKind kind);

} // namespace KernelLogic
