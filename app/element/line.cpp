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
    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName("LINE");
    setToolTip("LINE");
}

void Line::setSkin(bool defaultSkin, const QString &filename)
{
    m_pixmapSkinName[0] = defaultSkin ? ":/line.png" : filename;
    setPixmap(m_pixmapSkinName[0]);
}
