
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "line.h"

#include "globalproperties.h"

namespace
{
    int id = qRegisterMetaType<Line>();
}

Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::Line, ElementGroup::Other, ":/misc/line.svg", tr("LINE"), tr("Line"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setHasLabel(true);
}
