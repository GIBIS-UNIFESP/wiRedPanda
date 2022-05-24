// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mux.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Mux>();
}

Mux::Mux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Mux, ElementGroup::Mux, 3, 3, 1, 1, parent)
{
    m_defaultSkins = QStringList{":/basic/mux.png"};
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    Mux::updatePorts();
    setPortName("MUX");
    setToolTip(m_translatedName);
    setCanChangeSkin(true);

    input(0)->setName("0");
    input(1)->setName("1");
    input(2)->setName("S");
}

void Mux::updatePorts()
{
    input(0)->setPos(16, 32 - 12); /* 0   */
    input(1)->setPos(16, 32 + 12); /* 1   */
    input(2)->setPos(32, 58);      /* S   */

    output(0)->setPos(48, 32);     /* Out */
}
