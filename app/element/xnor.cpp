// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xnor.h"

Xnor::Xnor(QGraphicsItem *parent)
    : GraphicElement(ElementType::XNOR, ElementGroup::GATE, 2, 8, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/xnor.png"};

    setOutputsOnTop(true);
    setCanChangeSkin(true);
    setPixmap(m_pixmapSkinName[0]);
    updatePorts();
    setPortName("XNOR");
}

void Xnor::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/xnor.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
