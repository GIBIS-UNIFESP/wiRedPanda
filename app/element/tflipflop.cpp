// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tflipflop.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<TFlipFlop>();
}

TFlipFlop::TFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::TFlipFlop, ElementGroup::Memory, 4, 4, 2, 2, parent)
{
    if (GlobalProperties::skipInit) { return; }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setRotatable(false);
    setCanChangeSkin(true);
    TFlipFlop::updatePorts();
    setPortName("FlipFlop T");
    setToolTip(m_translatedName);

    input(0)->setName("T");
    input(1)->setName("Clock");
    input(2)->setName("~Preset");
    input(3)->setName("~Clear");

    output(0)->setName("Q");
    output(1)->setName("~Q");

    input(0)->setRequired(false);
    input(2)->setRequired(false);
    input(3)->setRequired(false);

    input(2)->setDefaultValue(Status::Active);
    input(3)->setDefaultValue(Status::Active);

    output(0)->setDefaultValue(Status::Inactive);
    output(1)->setDefaultValue(Status::Active);
}

void TFlipFlop::updatePorts()
{
    input(0)->setPos(leftPosition(), 13);   /* T */
    input(1)->setPos(leftPosition(), 45);   /* Clock */
    input(2)->setPos(32, leftPosition());   /* Preset */
    input(3)->setPos(32, rightPosition());  /* Clear */

    output(0)->setPos(rightPosition(), 15); /* Q */
    output(1)->setPos(rightPosition(), 45); /* ~Q */
}

void TFlipFlop::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
