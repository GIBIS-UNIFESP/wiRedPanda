// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xnor.h"

#include "globalproperties.h"

namespace
{
int id = qRegisterMetaType<Xnor>();
}

Xnor::Xnor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xnor, ElementGroup::Gate, ":/basic/xnor.svg", tr("<b>XNOR</b>"), tr("Xnor"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    updatePortsProperties();
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}
