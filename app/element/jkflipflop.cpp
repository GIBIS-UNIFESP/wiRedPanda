// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jkflipflop.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<JKFlipFlop>();
}

JKFlipFlop::JKFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::JKFlipFlop, ElementGroup::Memory, 5, 5, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setRotatable(false);
    setCanChangeSkin(true);
    JKFlipFlop::updatePorts();
    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    inputPort(0)->setName("J");
    inputPort(1)->setName("Clock");
    inputPort(2)->setName("K");
    inputPort(3)->setName("~Preset");
    inputPort(4)->setName("~Clear");

    outputPort(0)->setName("Q");
    outputPort(1)->setName("~Q");

    inputPort(0)->setRequired(false); /* J */
    inputPort(2)->setRequired(false); /* K */
    inputPort(3)->setRequired(false); /* p */
    inputPort(4)->setRequired(false); /* c */

    inputPort(0)->setDefaultValue(Status::Active);
    inputPort(2)->setDefaultValue(Status::Active);
    inputPort(3)->setDefaultValue(Status::Active);
    inputPort(4)->setDefaultValue(Status::Active);

    outputPort(0)->setDefaultValue(Status::Inactive);
    outputPort(1)->setDefaultValue(Status::Active);
}

void JKFlipFlop::updatePorts()
{
    inputPort(0)->setPos(0, 13);   /* J      */
    inputPort(1)->setPos(0, 29);   /* Clk    */
    inputPort(2)->setPos(0, 45);   /* K      */
    inputPort(3)->setPos(32, 0);   /* Preset */
    inputPort(4)->setPos(32, 64);  /* Clear  */

    outputPort(0)->setPos(64, 15); /* Q      */
    outputPort(1)->setPos(64, 45); /* ~Q     */
}

void JKFlipFlop::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
