// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Line.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Line> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Line,
        .group = ElementGroup::Other,
        .canChangeAppearance = true,
        // Label is enabled so users can attach a description string directly to the line.
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Misc/line.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Line", "LINE");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Line", "Line");
        descriptor.trContext = "Line";
        // Seed appearance lists from the constructor-supplied pixmap path (see And.cpp for details).
        descriptor.defaultAppearances = QStringList({":/Components/Misc/line.svg"});
        // Line is decorative and excluded from simulation.
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Line(); });
        return true;
    }();
};

// Line is a purely decorative element: 0 inputs and 0 outputs mean it has no
// simulation role. It exists so users can draw visual separators and annotation
// lines on the canvas.
Line::Line(QGraphicsItem *parent)
    : Element(ElementType::Line, parent)
{
}
