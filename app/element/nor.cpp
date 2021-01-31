// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nor.h"

Nor::Nor(QGraphicsItem *parent)
    : GraphicElement(ElementType::NOR, ElementGroup::GATE, 2, 8, 1, 1, parent)
{
    pixmapSkinName = {":/basic/nor.png"};
    setOutputsOnTop(true);
    setPixmap(pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("NOR");
}

void Nor::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/basic/nor.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
