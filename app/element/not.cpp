// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "not.h"

namespace
{
int id = qRegisterMetaType<Not>();
}

Not::Not(QGraphicsItem *parent)
    : GraphicElement(ElementType::Not, ElementGroup::Gate, 1, 1, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    updatePorts();
    setPortName("NOT");
    setToolTip(m_translatedName);
}
