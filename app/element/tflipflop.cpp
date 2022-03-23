// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tflipflop.h"

#include "qneport.h"

TFlipFlop::TFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::TFLIPFLOP, ElementGroup::MEMORY, 4, 4, 2, 2, parent)
{
    m_pixmapSkinName = {":/memory/T-flipflop.png"};

    setPixmap(m_pixmapSkinName[0]);
    setRotatable(false);
    setCanChangeSkin(true);
    TFlipFlop::updatePorts();
    lastClk = false;
    lastT = 0;
    lastQ = 0;
    setPortName("FlipFlop T");

    input(0)->setName("T");
    input(1)->setName("Clock");
    input(2)->setName("~Preset");
    input(3)->setName("~Clear");
    output(0)->setName("Q");
    output(1)->setName("~Q");

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);

    input(0)->setRequired(false);
    input(2)->setRequired(false);
    input(3)->setRequired(false);
    input(2)->setDefaultValue(1);
    input(3)->setDefaultValue(1);
}

void TFlipFlop::updatePorts()
{
    input(0)->setPos(topPosition(), 13); /* T */
    input(1)->setPos(topPosition(), 45); /* Clock */
    input(2)->setPos(32, topPosition()); /* Preset */
    input(3)->setPos(32, bottomPosition()); /* Clear */

    output(0)->setPos(bottomPosition(), 15); /* Q */
    output(1)->setPos(bottomPosition(), 45); /* ~Q */
}

void TFlipFlop::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/memory/T-flipflop.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
