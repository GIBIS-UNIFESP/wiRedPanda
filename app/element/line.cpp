// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "line.h"

Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::LINE, ElementGroup::OTHER, 0, 0, 0, 0, parent)
{
    m_pixmapSkinName = {":/line.png"};

    setRotatable(true);
    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    setCanChangeSkin(false);
    setHasLabel(true);
    setPortName("LINE");
    setToolTip("LINE");
}
