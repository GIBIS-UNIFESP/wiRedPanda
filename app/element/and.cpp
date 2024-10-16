// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "and.h"

#include "globalproperties.h"

namespace
{
    int id = qRegisterMetaType<And>();
}

And::And(QGraphicsItem *parent)
    : GraphicElement(ElementType::And, ElementGroup::Gate, ":/basic/and.svg", tr("AND"), tr("And"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
}
