// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputgnd.h"

#include "qneport.h"

InputGnd::InputGnd(QGraphicsItem *parent)
    : GraphicElement(ElementType::Gnd, ElementGroup::StaticInput, 0, 0, 1, 1, parent)
{
    m_pixmapSkinName = {":/input/0.png"};

    setOutputsOnTop(false);
    setCanChangeSkin(true);
    setPixmap(m_pixmapSkinName[0]);
    setRotatable(false);
    setPortName("GND");
    m_outputs.first()->setValue(false);
}

void InputGnd::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/input/0.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
