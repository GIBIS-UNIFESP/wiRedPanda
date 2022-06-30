// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nand.h"

namespace
{
int id = qRegisterMetaType<Nand>();
}

Nand::Nand(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nand, ElementGroup::Gate, ":/basic/nand.svg", tr("<b>NAND</b>"), tr("Nand"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    updatePorts();
    setCanChangeSkin(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}
