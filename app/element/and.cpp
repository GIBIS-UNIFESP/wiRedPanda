// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "and.h"

And::And(QGraphicsItem *parent)
    : GraphicElement(ElementType::AND, ElementGroup::GATE, 2, 8, 1, 1, parent)
{
    pixmapSkinName = {":/basic/and.png"};

    setOutputsOnTop(true);
    setPixmap(pixmapSkinName[0]);
    updatePorts();
    setCanChangeSkin(true);
    setPortName("AND");
    setToolTip("AND");
}

void And::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin)
        pixmapSkinName[0] = ":/basic/and.png";
    else
        pixmapSkinName[0] = filename;
    setPixmap(pixmapSkinName[0]);
}
