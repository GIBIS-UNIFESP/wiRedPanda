// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text.h"

namespace {
int id = qRegisterMetaType<Text>();
}

Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, ElementGroup::Other, 0, 0, 0, 0, parent)
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
    setToolTip(m_translatedName);
}
