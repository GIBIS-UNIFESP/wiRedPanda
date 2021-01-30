// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mux.h"

Mux::Mux(QGraphicsItem *parent)
    : GraphicElement(3, 3, 1, 1, parent)
{
    pixmapSkinName.append(":/basic/mux.png");
    setPixmap(pixmapSkinName[0]);
    setRotatable(true);
    updatePorts();
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

void Mux::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin)
        pixmapSkinName[0] = ":/basic/mux.png";
    else
        pixmapSkinName[0] = filename;
    setPixmap(pixmapSkinName[0]);
}
