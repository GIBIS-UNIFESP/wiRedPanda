// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "line.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Line>();
}

Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::Line, ElementGroup::Other, 0, 0, 0, 0, parent)
{
    m_defaultSkins = QStringList{":/line.png"};
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName("LINE");
    setToolTip(m_translatedName);
}
