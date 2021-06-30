// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text.h"

Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::TEXT, ElementGroup::OTHER, 0, 0, 0, 0, parent)
{
    m_pixmapSkinName = {
        ":/no_text.png",
        ":/text.png",
    };
    setRotatable(true);
    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    setCanChangeSkin(false);
    setHasLabel(true);
    setPortName("TEXT");
    setToolTip("TEXT");
}
