// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputGND.h"

#include "App/GlobalProperties.h"

InputGnd::InputGnd(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputGnd, ElementGroup::StaticInput, ":/Components/Input/0.svg", tr("GROUND"), tr("GND"), 0, 0, 1, 1, parent)
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
    // Static inputs are not rotatable as a whole; instead their output port rotates
    // around the element centre so the wire attachment point tracks the correct position.
    setRotatable(false);
}

