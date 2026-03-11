// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Line.h"

#include "App/GlobalProperties.h"

// Line is a purely decorative element: 0 inputs and 0 outputs mean it has no
// simulation role and is mapped to LogicNone by ElementFactory::buildLogicElement().
// It exists so users can draw visual separators and annotation lines on the canvas.
Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::Line, ElementGroup::Other, ":/Components/Misc/line.svg", tr("LINE"), tr("Line"), 0, 0, 0, 0, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
    if (GlobalProperties::skipInit) {
        return;
    }

    // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    // Label is enabled so users can attach a description string directly to the line.
    setHasLabel(true);
}

