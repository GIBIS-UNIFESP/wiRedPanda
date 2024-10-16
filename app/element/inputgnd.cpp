// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputgnd.h"

#include "globalproperties.h"

namespace
{
    int id = qRegisterMetaType<InputGnd>();
}

InputGnd::InputGnd(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputGnd, ElementGroup::StaticInput, ":/input/0.svg", tr("GROUND"), tr("GND"), 0, 0, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setRotatable(false);
}
