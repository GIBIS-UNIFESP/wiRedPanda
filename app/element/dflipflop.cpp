// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dflipflop.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<DFlipFlop>();
}

DFlipFlop::DFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::DFlipFlop, ElementGroup::Memory, 4, 4, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setRotatable(false);
    setCanChangeSkin(true);
    DFlipFlop::updatePorts();
    setPortName("FlipFlop D");
    setToolTip(m_translatedName);

    inputPort(0)->setName("Data");
    inputPort(1)->setName("Clock");
    inputPort(2)->setName("~Preset");
    inputPort(3)->setName("~Clear");

    outputPort(0)->setName("Q");
    outputPort(1)->setName("~Q");

    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);

    inputPort(2)->setDefaultValue(Status::Active);
    inputPort(3)->setDefaultValue(Status::Active);

    outputPort(0)->setDefaultValue(Status::Inactive);
    outputPort(1)->setDefaultValue(Status::Active);
}

void DFlipFlop::updatePorts()
{
    inputPort(0)->setPos(leftPosition(), 13);   /* Data   */
    inputPort(1)->setPos(leftPosition(), 45);   /* Clock  */
    inputPort(2)->setPos(32, leftPosition());   /* Preset */
    inputPort(3)->setPos(32, rightPosition());  /* Clear  */

    outputPort(0)->setPos(rightPosition(), 15); /* Q      */
    outputPort(1)->setPos(rightPosition(), 45); /* ~Q     */
}

void DFlipFlop::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
