// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "not.h"

#include "globalproperties.h"

Not::Not(QGraphicsItem *parent)
    : GraphicElement(ElementType::Not, ElementGroup::Gate, ":/components/logic/not.svg", tr("NOT"), tr("Not"), 1, 1, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
}
