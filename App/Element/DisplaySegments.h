// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DisplaySegment and DisplaySegments: the segment inputs of the 7-, 14- and 16-segment displays.
 */

#pragma once

#include <optional>
#include <span>

#include <QtGlobal>

#include "App/Element/PortAttributes.h"

class Element;

/// One segment input of a display: where it sits, its letter and its description, marked with QT_TRANSLATE_NOOP() in
/// the display type's translation context (ElementTypeDescriptor::trContext). The strings outlive the element, as
/// string literals do.
struct DisplaySegment {
    qreal x;
    qreal y;
    const char *letter;
    const char *description;
};

namespace DisplaySegments {

/**
 * \brief Returns the attributes of segment input \a index of \a display, whose segments \a segments lists in port
 * order; nothing for an index past them.
 * \details The input is named by its letter and its description, translated in the display's type context each time
 * the name is asked for, so it follows the language, with its source in English. Segments are individually driven: the
 * input is optional, and leaving it unconnected turns the segment off.
 */
std::optional<PortAttributes> input(const Element &display, std::span<const DisplaySegment> segments, int index);

} // namespace DisplaySegments
