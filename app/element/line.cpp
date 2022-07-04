// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "line.h"

#include "globalproperties.h"

namespace
{
int id = qRegisterMetaType<Line>();
}

Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::Line, ElementGroup::Other, ":/misc/line.svg", tr("<b>LINE</b>"), tr("Line"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}
