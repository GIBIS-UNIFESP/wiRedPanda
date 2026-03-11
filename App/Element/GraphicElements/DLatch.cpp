// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/DLatch.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

DLatch::DLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::DLatch, ElementGroup::Memory, pixmapPath(), tr("D-LATCH"), tr("D-Latch"), 2, 2, 2, 2, parent)
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
    DLatch::updatePortsProperties();
}

void DLatch::updatePortsProperties()
{
    // D latch has no clock — it is level-sensitive: Q tracks D while Enable is HIGH.
    // No asynchronous preset/clear inputs, so only 2 inputs total.
    inputPort(0)->setPos(0, 16);      inputPort(0)->setName("Data");
    inputPort(1)->setPos(0, 48);      inputPort(1)->setName("Enable");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void DLatch::updateTheme()
{
    // Reload the pixmap before delegating to the base class (see SRFlipFlop.cpp).
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
