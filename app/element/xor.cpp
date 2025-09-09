// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xor.h"

#include "globalproperties.h"

Xor::Xor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xor, ElementGroup::Gate, ":/components/logic/xor.svg", tr("XOR"), tr("Xor"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
}
