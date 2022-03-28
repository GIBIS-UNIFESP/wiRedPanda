// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xor.h"

Xor::Xor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_pixmapSkinName.append(":/basic/xor.png");
    setOutputsOnTop(true);
    setCanChangeSkin(true);
    setPixmap(m_pixmapSkinName[0]);
    updatePorts();
    setPortName("XOR");
}

void Xor::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/xor.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
