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
    m_defaultSkins = QStringList{
        ":/no_text.png",
        ":/text.png",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setHasLabel(true);
    setPortName("TEXT");
    setToolTip(m_translatedName);
}
