// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text.h"

#include "globalproperties.h"

namespace
{
int id = qRegisterMetaType<Text>();
}

Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, ElementGroup::Other, ":/misc/text.png", tr("<b>TEXT</b>"), tr("Text"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/misc/no_text.png",
        ":/misc/text.png",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setHasLabel(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}
