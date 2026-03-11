// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text.h"

#include "globalproperties.h"

Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, ElementGroup::Other, ":/components/misc/text.png", tr("TEXT"), tr("Text"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/components/misc/no_text.png",
        ":/components/misc/text.png",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setHasLabel(true);
}
