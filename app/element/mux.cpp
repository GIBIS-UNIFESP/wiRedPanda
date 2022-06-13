// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mux.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Mux>();
}

Mux::Mux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Mux, ElementGroup::Mux, 3, 3, 1, 1, parent)
{
    if (GlobalProperties::skipInit) { return; }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    Mux::updatePorts();
    setPortName("MUX");
    setToolTip(m_translatedName);
    setCanChangeSkin(true);

    inputPort(0)->setName("0");
    inputPort(1)->setName("1");
    inputPort(2)->setName("S");
}

void Mux::updatePorts()
{
    inputPort(0)->setPos(16, 32 - 12); /* 0   */
    inputPort(1)->setPos(16, 32 + 12); /* 1   */
    inputPort(2)->setPos(32, 58);      /* S   */

    outputPort(0)->setPos(48, 32);     /* Out */
}
