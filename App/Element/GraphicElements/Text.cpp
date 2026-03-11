// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Text.h"

#include "GlobalProperties.h"

Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, ElementGroup::Other, ":/Components/Misc/text.png", tr("TEXT"), tr("Text"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/Components/Misc/no_text.png",
        ":/Components/Misc/text.png",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setHasLabel(true);
}
