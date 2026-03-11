// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/JKFlipFlop.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

JKFlipFlop::JKFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::JKFlipFlop, ElementGroup::Memory, pixmapPath(), tr("JK-FLIP-FLOP"), tr("JK-Flip-Flop"), 5, 5, 2, 2, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
    if (GlobalProperties::skipInit) {
        return;
    }

    // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    // Call the most-derived override explicitly (see SRFlipFlop.cpp for rationale).
    JKFlipFlop::updatePortsProperties();
}

void JKFlipFlop::updatePortsProperties()
{
    // JK flip-flop: J (Set-like) and K (Reset-like) are clocked, with Clock required.
    // J=1,K=1 on a clock edge toggles output — making the JK superior to SR (no forbidden state).
    // ~Preset and ~Clear are asynchronous active-low overrides (same as DFlipFlop).
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("J");
    inputPort(1)->setPos( 0, 32);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos( 0, 48);     inputPort(2)->setName("K");
    inputPort(3)->setPos(32,  0);     inputPort(3)->setName("~Preset");
    inputPort(4)->setPos(32, 64);     inputPort(4)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    inputPort(0)->setRequired(false);
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);
    inputPort(4)->setRequired(false);

    // J and K default to Active (HIGH) when unconnected, so the flip-flop toggles on every
    // clock edge by default — a useful demonstration behavior for the educational context.
    // ~Preset and ~Clear default to Active (HIGH = not asserted).
    inputPort(0)->setDefaultStatus(Status::Active);
    inputPort(2)->setDefaultStatus(Status::Active);
    inputPort(3)->setDefaultStatus(Status::Active);
    inputPort(4)->setDefaultStatus(Status::Active);

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void JKFlipFlop::updateTheme()
{
    // Reload the pixmap before delegating to the base class (see SRFlipFlop.cpp).
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
