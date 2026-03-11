// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Nand.h"

#include "App/GlobalProperties.h"

Nand::Nand(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nand, ElementGroup::Gate, ":/Components/Logic/nand.svg", tr("NAND"), tr("Nand"), 2, 8, 1, 1, parent)
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
}
