// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mux.h"

#include "qneport.h"

Mux::Mux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Mux, ElementGroup::Mux, 3, 3, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/mux.png"};

    setPixmap(m_pixmapSkinName[0]);
    setRotatable(true);
    Mux::updatePorts();
    setPortName("MUX");
    setCanChangeSkin(true);
    input(0)->setName("0");
    input(1)->setName("1");
    input(2)->setName("S");
}

void Mux::updatePorts()
{
    input(0)->setPos(32 - 12, 48); /* 0 */
    input(1)->setPos(32 + 12, 48); /* 1 */
    input(2)->setPos(58, 32); /* S */
    output(0)->setPos(32, 16); /* Out */
}

void Mux::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/mux.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
