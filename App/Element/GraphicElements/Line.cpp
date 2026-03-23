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
        .minInputSize = 0,
        .maxInputSize = 0,
        .minOutputSize = 0,
        .maxOutputSize = 0,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        // Label is enabled so users can attach a description string directly to the line.
        .hasLabel = true,
        .hasTruthTable = false,
        .rotatable = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Misc/line.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Line", "LINE");
        meta.translatedName = QT_TRANSLATE_NOOP("Line", "Line");
        meta.trContext = "Line";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Misc/line.svg"});
        // Line is decorative and excluded from simulation.
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Line(); });
        return true;
    }();
};

// Line is a purely decorative element: 0 inputs and 0 outputs mean it has no
// simulation role. It exists so users can draw visual separators and annotation
// lines on the canvas.
Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::Line, parent)
{
}

