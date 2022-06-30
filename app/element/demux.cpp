// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "demux.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Demux>();
}

Demux::Demux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Demux, ElementGroup::Mux, ":/basic/demux.svg", tr("<b>DEMULTIPLEXER</b>"), tr("Demux"), 2, 2, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    Demux::updatePorts();
}

void Demux::updatePorts()
{
    inputPort(0)->setPos(16, 32);     inputPort(0)->setName("In");
    inputPort(1)->setPos(32, 56);     inputPort(1)->setName("S");

    outputPort(0)->setPos(48, 16);    outputPort(0)->setName("Out0");
    outputPort(1)->setPos(48, 48);    outputPort(1)->setName("Out1");
}
