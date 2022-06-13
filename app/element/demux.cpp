// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "demux.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Demux>();
}

Demux::Demux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Demux, ElementGroup::Mux, 2, 2, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    Demux::updatePorts();
    setPortName("DEMUX");
    setToolTip(m_translatedName);

    inputPort(0)->setName("in");
    inputPort(1)->setName("S");

    outputPort(0)->setName("out0");
    outputPort(1)->setName("out1");
}

void Demux::updatePorts()
{
    inputPort(0)->setPos(16, 32);       /* 0   */
    inputPort(1)->setPos(32, 58);       /* S   */

    outputPort(0)->setPos(48, 32 - 12); /* Out */
    outputPort(1)->setPos(48, 32 + 12); /* Out */
}
