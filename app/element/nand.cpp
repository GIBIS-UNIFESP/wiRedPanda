// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nand.h"

Nand::Nand(QGraphicsItem *parent)
    : GraphicElement(ElementType::NAND, ElementGroup::GATE, 2, 8, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/nand.png"};

    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setCanChangeSkin(true);
    setPortName("NAND");
}

void Nand::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/nand.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
