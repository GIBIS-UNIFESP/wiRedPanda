// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "or.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Or>();
}

Or::Or(QGraphicsItem *parent)
    : GraphicElement(ElementType::Or, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    updatePorts();
    setPortName("OR");
    setToolTip(m_translatedName);
}
