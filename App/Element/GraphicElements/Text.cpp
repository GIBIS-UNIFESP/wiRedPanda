// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Text.h"

#include "App/GlobalProperties.h"

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
