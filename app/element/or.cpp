// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "or.h"

Or::Or(QGraphicsItem *parent)
    : GraphicElement(ElementType::OR, ElementGroup::GATE, 2, 8, 1, 1, parent)
{
    pixmapSkinName = {":/basic/or.png"};

    setOutputsOnTop(true);
    setPixmap(pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("OR");
}

void Or::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/basic/or.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
