// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/And.h"

#include "App/GlobalProperties.h"

And::And(QGraphicsItem *parent)
    : GraphicElement(ElementType::And, ElementGroup::Gate, ":/Components/Logic/and.svg", tr("AND"), tr("And"), 2, 8, 1, 1, parent)
{
    // skipInit is set by ElementFactory::ensurePropertiesCached() when it needs
    // to build a throw-away instance just to read static Q_PROPERTY values.
    // Returning early avoids expensive pixmap loads and port allocation for that case.
    if (GlobalProperties::skipInit) {
        return;
    }

    // m_pixmapPath was set by the base-class constructor from the argument above.
    // Seed both skin lists from it so that index 0 always points to the default art.
    // m_alternativeSkins starts as a copy of m_defaultSkins; the user can replace
    // entries in m_alternativeSkins without losing the originals in m_defaultSkins.
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
}
