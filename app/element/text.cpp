// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text.h"

namespace
{
int id = qRegisterMetaType<Text>();
}

Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, ElementGroup::Other, 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) { return; }

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
