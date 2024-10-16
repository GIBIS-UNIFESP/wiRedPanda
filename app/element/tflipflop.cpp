// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tflipflop.h"

#include "globalproperties.h"
#include "qneport.h"

namespace
{
    int id = qRegisterMetaType<TFlipFlop>();
}

TFlipFlop::TFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::TFlipFlop, ElementGroup::Memory, pixmapPath(), tr("T-FLIP-FLOP"), tr("T-Flip-Flop"), 4, 4, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    TFlipFlop::updatePortsProperties();
}

void TFlipFlop::updatePortsProperties()
{
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("T");
    inputPort(1)->setPos( 0, 48);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos(32,  0);     inputPort(2)->setName("~Preset");
    inputPort(3)->setPos(32, 64);     inputPort(3)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    inputPort(0)->setRequired(false);
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);

    inputPort(2)->setDefaultStatus(Status::Active);
    inputPort(3)->setDefaultStatus(Status::Active);

    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void TFlipFlop::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
