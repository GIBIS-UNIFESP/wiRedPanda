// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Text>();
}

Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, ElementGroup::Other, 0, 0, 0, 0, parent)
{
    qCDebug(zero) << "Creating text.";

    m_pixmapSkinName = QStringList{
        ":/no_text.png",
        ":/text.png",
    };
    setPixmap(m_pixmapSkinName.first());

    setRotatable(true);
    setOutputsOnTop(true);
    setCanChangeSkin(false);
    setHasLabel(true);
    setPortName("TEXT");
    setToolTip(m_translatedName);
}
