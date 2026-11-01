// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/DisplaySegments.h"

#include <utility>

#include <QCoreApplication>

#include "App/Element/Element.h"

std::optional<PortAttributes> DisplaySegments::input(const Element &display,
                                                     const std::span<const DisplaySegment> segments,
                                                     const int index)
{
    if ((index < 0) || (std::cmp_greater_equal(index, segments.size()))) {
        return std::nullopt;
    }
    const DisplaySegment &segment = segments[static_cast<std::size_t>(index)];

    // One builder for the shown name and its source, so the two differ only by the description's translation
    const auto described = [letter = segment.letter](const QString &text) {
        return QString::fromUtf8(letter) + " (" + text + ')';
    };
    const char *context = display.descriptor().trContext;
    const char *description = segment.description;
    return PortAttributes{
        .derivedName = PortAttributes::DerivedName{
            [described, context, description] { return described(QCoreApplication::translate(context, description)); },
            described(QString::fromUtf8(description)),
        },
        .required = false,
        .defaultStatus = Status::Inactive,
        .position = QPointF(segment.x, segment.y),
    };
}
