// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dflipflop.h"

#include "globalproperties.h"
#include "qneport.h"

namespace
{
    int id = qRegisterMetaType<DFlipFlop>();
}

DFlipFlop::DFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::DFlipFlop, ElementGroup::Memory, pixmapPath(), tr("D-FLIPFLOP"), tr("D-FlipFlop"), 4, 4, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    DFlipFlop::updatePortsProperties();
}

void DFlipFlop::updatePortsProperties()
{
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("Data");
    inputPort(1)->setPos( 0, 48);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos(32,  0);     inputPort(2)->setName("~Preset");
    inputPort(3)->setPos(32, 64);     inputPort(3)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);

    inputPort(2)->setDefaultStatus(Status::Active);
    inputPort(3)->setDefaultStatus(Status::Active);

    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void DFlipFlop::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
