// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xor.h"

namespace
{
int id = qRegisterMetaType<Xor>();
}

Xor::Xor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xor, ElementGroup::Gate, ":/basic/xor.svg", tr("<b>XOR</b>"), tr("Xor"), 2, 8, 1, 1, parent)
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
