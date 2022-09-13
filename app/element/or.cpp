// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "or.h"

#include "globalproperties.h"

namespace
{
int id = qRegisterMetaType<Or>();
}

Or::Or(QGraphicsItem *parent)
    : GraphicElement(ElementType::Or, ElementGroup::Gate, ":/basic/or.svg", tr("OR"), tr("Or"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    updatePortsProperties();
}
