// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlatch.h"

DLatch::DLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::DLATCH, ElementGroup::MEMORY, 2, 2, 2, 2, parent)
{
    pixmapSkinName = {":/memory/D-latch.png"};

    setPixmap(pixmapSkinName[0]);
    setRotatable(false);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("D Latch");
    input(0)->setName("Data");
    input(1)->setName("Enable");
    output(0)->setName("Q");
    output(1)->setName("~Q");

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);
}

void DLatch::updatePorts()
{
    input(0)->setPos(topPosition(), 13); /* Data */
    input(1)->setPos(topPosition(), 45); /* Enable */

    output(0)->setPos(bottomPosition(), 15); /* Q */
    output(1)->setPos(bottomPosition(), 45); /* ~Q */
}

void DLatch::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/memory/D-latch.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
