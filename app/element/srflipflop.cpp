// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "srflipflop.h"

#include "globalproperties.h"
#include "qneport.h"

namespace
{
    int id = qRegisterMetaType<SRFlipFlop>();
}

SRFlipFlop::SRFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::SRFlipFlop, ElementGroup::Memory, pixmapPath(), tr("SR-FLIPFLOP"), tr("SR-FlipFlop"), 5, 5, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    SRFlipFlop::updatePortsProperties();
}

void SRFlipFlop::updatePortsProperties()
{
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("S");
    inputPort(1)->setPos( 0, 32);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos( 0, 48);     inputPort(2)->setName("R");
    inputPort(3)->setPos(32,  0);     inputPort(3)->setName("~Preset");
    inputPort(4)->setPos(32, 64);     inputPort(4)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    inputPort(0)->setRequired(false);
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);
    inputPort(4)->setRequired(false);

    inputPort(3)->setDefaultStatus(Status::Active);
    inputPort(4)->setDefaultStatus(Status::Active);

    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void SRFlipFlop::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
