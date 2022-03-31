// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "line.h"

Line::Line(QGraphicsItem *parent)
    : GraphicElement(ElementType::Line, ElementGroup::Other, 0, 0, 0, 0, parent)
{
    m_pixmapSkinName = {":/line.png"};

    setRotatable(true);
    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName("LINE");
}

void Line::setSkin(bool defaultSkin, const QString &filename)
{
    m_pixmapSkinName[0] = defaultSkin ? ":/line.png" : filename;
    setPixmap(m_pixmapSkinName[0]);
}
