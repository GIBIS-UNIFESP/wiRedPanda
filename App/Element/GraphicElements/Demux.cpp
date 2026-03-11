// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Demux.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

Demux::Demux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Demux, ElementGroup::Mux, ":/Components/Logic/demux.svg", tr("DEMULTIPLEXER"), tr("Demux"), 2, 2, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    Demux::updatePortsProperties();
}

void Demux::updatePortsProperties()
{
    // The 1-of-2 demux SVG is 64×64 px. Port coordinates are in item-local space:
    //   "In"  — data input, left side, vertically centred (y=32)
    //   "S"   — select line, lower-left corner (x=32, y=56); offset right so the
    //            wire doesn't overlap the data-input wire at the same x position
    //   "Out0"/"Out1" — right side at y=16/48, evenly splitting the body height
    inputPort(0)->setPos(16, 32);     inputPort(0)->setName("In");
    inputPort(1)->setPos(32, 56);     inputPort(1)->setName("S");

    outputPort(0)->setPos(48, 16);    outputPort(0)->setName("Out0");
    outputPort(1)->setPos(48, 48);    outputPort(1)->setName("Out1");
}

