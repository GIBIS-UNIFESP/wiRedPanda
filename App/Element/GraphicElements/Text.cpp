// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Text.h"

#include "App/GlobalProperties.h"

// Text is a purely decorative annotation element with no ports and no simulation role.
// It is mapped to LogicNone by ElementFactory::buildLogicElement().
Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, ElementGroup::Other, ":/Components/Misc/text.png", tr("TEXT"), tr("Text"), 0, 0, 0, 0, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
    if (GlobalProperties::skipInit) {
        return;
    }

    // Text has two built-in skins:
    //   index 0 — transparent placeholder shown when the label is empty
    //   index 1 — solid background shown when the label has content
    // Unlike other elements, the skin list is initialised with a QStringList literal
    // rather than appending m_pixmapPath, because the two paths differ from the
    // constructor argument (which is just used for the palette thumbnail).
    m_defaultSkins = QStringList{
        ":/Components/Misc/no_text.png",
        ":/Components/Misc/text.png",
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    // Label must be visible by default because the text body IS the label content.
    setHasLabel(true);
}

QRectF Text::boundingRect() const
{
    return GraphicElement::boundingRect().united(childrenBoundingRect());
}

