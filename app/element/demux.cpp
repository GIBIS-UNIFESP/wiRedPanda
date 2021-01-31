// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "demux.h"

Demux::Demux(QGraphicsItem *parent)
    : GraphicElement(ElementType::DEMUX, ElementGroup::MUX, 2, 2, 2, 2, parent)
{
    pixmapSkinName.append(":/basic/demux.png");
    setPixmap(pixmapSkinName[0]);
    setRotatable(true);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("DEMUX");
    setRotation(180.0);

    input(0)->setName("in");
    input(1)->setName("S");

    output(0)->setName("out0");
    output(1)->setName("out1");
}

void Demux::updatePorts()
{
    input(0)->setPos(32, 48); /* 0 */
    input(1)->setPos(58, 32); /* S */
    output(0)->setPos(32 - 12, 16); /* Out */
    output(1)->setPos(32 + 12, 16); /* Out */
}

void Demux::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/basic/demux.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
