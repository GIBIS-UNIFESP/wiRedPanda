// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "not.h"

Not::Not(QGraphicsItem *parent)
    : GraphicElement(ElementType::NOT, ElementGroup::GATE, 1, 1, 1, 1, parent)
{
    pixmapSkinName = {":/basic/not.png"};
    setOutputsOnTop(true);
    setPixmap(pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("NOT");
}

void Not::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/basic/not.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
