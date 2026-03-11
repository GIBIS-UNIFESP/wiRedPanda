// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Mux.h"

#include "GlobalProperties.h"
#include "QNEPort.h"

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
    inputPort(0)->setPos(16, 16); inputPort(0)->setName("In0");
    inputPort(1)->setPos(16, 48); inputPort(1)->setName("In1");
    inputPort(2)->setPos(32, 56); inputPort(2)->setName("S");

    outputPort(0)->setPos(48, 32); outputPort(0)->setName("Out");
}
