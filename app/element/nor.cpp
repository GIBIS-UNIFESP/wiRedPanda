
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "nor.h"

#include "globalproperties.h"

namespace
{
    int id = qRegisterMetaType<Nor>();
}

Nor::Nor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nor, ElementGroup::Gate, ":/basic/nor.svg", tr("NOR"), tr("Nor"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
}
