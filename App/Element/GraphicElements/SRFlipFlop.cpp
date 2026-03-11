// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/SRFlipFlop.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

SRFlipFlop::SRFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::SRFlipFlop, ElementGroup::Memory, pixmapPath(), tr("SR-FLIP-FLOP"), tr("SR-Flip-Flop"), 5, 5, 2, 2, parent)
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

    // Call the most-derived override explicitly to guarantee correct port positions
    // even if a subclass overrides updatePortsProperties() further.
    SRFlipFlop::updatePortsProperties();
}

void SRFlipFlop::updatePortsProperties()
{
    // SR flip-flop: S (Set) and R (Reset) are clocked inputs; Clock is required.
    // ~Preset and ~Clear provide asynchronous overrides (active-low, same as DFlipFlop).
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("S");
    inputPort(1)->setPos( 0, 32);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos( 0, 48);     inputPort(2)->setName("R");
    inputPort(3)->setPos(32,  0);     inputPort(3)->setName("~Preset");
    inputPort(4)->setPos(32, 64);     inputPort(4)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    // S, R, ~Preset, ~Clear are optional; only Clock is always required.
    // S and R unconnected default to Inactive (no set/reset action on clock edge).
    // ~Preset and ~Clear unconnected default to Active (HIGH = not asserted).
    inputPort(0)->setRequired(false);
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);
    inputPort(4)->setRequired(false);

    inputPort(3)->setDefaultStatus(Status::Active);
    inputPort(4)->setDefaultStatus(Status::Active);

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void SRFlipFlop::updateTheme()
{
    // Reload the pixmap before delegating to the base class so the correct
    // theme-specific SVG is used before ports and colours are updated.
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}

