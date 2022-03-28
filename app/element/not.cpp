// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "not.h"

Not::Not(QGraphicsItem *parent)
    : GraphicElement(ElementType::Not, ElementGroup::Gate, 1, 1, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/not.png"};
    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("NOT");
}

void Not::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/not.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
