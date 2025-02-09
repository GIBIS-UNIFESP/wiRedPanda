// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xnor.h"

#include "globalproperties.h"

Xnor::Xnor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xnor, ElementGroup::Gate, ":/basic/xnor.svg", tr("XNOR"), tr("Xnor"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
}
