// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Mux.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

Mux::Mux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Mux, ElementGroup::Mux, ":/Components/Logic/mux.svg", tr("MULTIPLEXER"), tr("Mux"), 3, 3, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    Mux::updatePortsProperties();
}

void Mux::updatePortsProperties()
{
    // The 2-to-1 mux SVG is 64×64 px. Port coordinates are in item-local space:
    //   "In0"/"In1" — left side data inputs at y=16/48, evenly splitting the body height
    //   "S"  — select line, lower-left area (x=32, y=56); offset right so the wire
    //           doesn't cross the data-input wires entering from the left edge
    //   "Out" — right side, vertically centred (y=32) relative to the two inputs
    inputPort(0)->setPos(16, 16); inputPort(0)->setName("In0");
    inputPort(1)->setPos(16, 48); inputPort(1)->setName("In1");
    inputPort(2)->setPos(32, 56); inputPort(2)->setName("S");

    outputPort(0)->setPos(48, 32); outputPort(0)->setName("Out");
}
