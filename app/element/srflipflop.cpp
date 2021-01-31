// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "srflipflop.h"

SRFlipFlop::SRFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::SRFLIPFLOP, ElementGroup::MEMORY, 5, 5, 2, 2, parent)
{
    pixmapSkinName = {":/memory/SR-flipflop.png"};
    setPixmap(pixmapSkinName[0]);
    setRotatable(false);
    setCanChangeSkin(true);
    updatePorts();
    lastClk = false;
    setPortName("FlipFlop SR");

    input(0)->setName("S");
    input(1)->setName("Clock");
    input(2)->setName("R");
    input(3)->setName("~Preset");
    input(4)->setName("~Clear");
    output(0)->setName("Q");
    output(1)->setName("~Q");

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);

    input(0)->setRequired(false); /* S */
    input(2)->setRequired(false); /* R */
    input(3)->setRequired(false); /* p */
    input(4)->setRequired(false); /* c */
    input(3)->setDefaultValue(1);
    input(4)->setDefaultValue(1);
}

void SRFlipFlop::updatePorts()
{
    input(0)->setPos(topPosition(), 13); /* S */
    input(1)->setPos(topPosition(), 29); /* Clk */
    input(2)->setPos(topPosition(), 45); /* R */
    input(3)->setPos(32, topPosition()); /* Preset */
    input(4)->setPos(32, bottomPosition()); /* Clear */

    output(0)->setPos(bottomPosition(), 15); /* Q */
    output(1)->setPos(bottomPosition(), 45); /* ~Q */
}

void SRFlipFlop::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/memory/SR-flipflop.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
