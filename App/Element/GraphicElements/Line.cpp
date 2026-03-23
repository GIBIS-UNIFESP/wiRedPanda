// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Line.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicNone.h"

template<>
struct ElementInfo<Line> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Line,
        .group = ElementGroup::Other,
        .canChangeSkin = true,
        // Label is enabled so users can attach a description string directly to the line.
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Misc/line.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Line", "LINE");
        meta.translatedName = QT_TRANSLATE_NOOP("Line", "Line");
        meta.trContext = "Line";
        meta.defaultSkins = QStringList({":/Components/Misc/line.svg"});
        meta.logicCreator = [](GraphicElement *) { return std::make_shared<LogicNone>(); };
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

