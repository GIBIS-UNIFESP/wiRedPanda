// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "srflipflop.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<SRFlipFlop>();
}

SRFlipFlop::SRFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::SRFlipFlop, ElementGroup::Memory, 5, 5, 2, 2, parent)
{
    if (GlobalProperties::skipInit) { return; }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setRotatable(false);
    setCanChangeSkin(true);
    SRFlipFlop::updatePorts();
    setPortName("FlipFlop SR");
    setToolTip(m_translatedName);

    input(0)->setName("S");
    input(1)->setName("Clock");
    input(2)->setName("R");
    input(3)->setName("~Preset");
    input(4)->setName("~Clear");

    output(0)->setName("Q");
    output(1)->setName("~Q");

    input(0)->setRequired(false); /* S */
    input(2)->setRequired(false); /* R */
    input(3)->setRequired(false); /* p */
    input(4)->setRequired(false); /* c */

    input(3)->setDefaultValue(1);
    input(4)->setDefaultValue(1);

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);
}

void SRFlipFlop::updatePorts()
{
    input(0)->setPos(leftPosition(), 13);   /* S */
    input(1)->setPos(leftPosition(), 29);   /* Clk */
    input(2)->setPos(leftPosition(), 45);   /* R */
    input(3)->setPos(32, leftPosition());   /* Preset */
    input(4)->setPos(32, rightPosition());  /* Clear */

    output(0)->setPos(rightPosition(), 15); /* Q */
    output(1)->setPos(rightPosition(), 45); /* ~Q */
}

void SRFlipFlop::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
