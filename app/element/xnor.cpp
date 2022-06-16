// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xnor.h"

namespace
{
int id = qRegisterMetaType<Xnor>();
}

Xnor::Xnor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xnor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    updatePorts();
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}
