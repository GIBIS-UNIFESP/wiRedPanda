// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputvcc.h"

#include "qneport.h"

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::Vcc, ElementGroup::StaticInput, 0, 0, 1, 1, parent)
{
    m_pixmapSkinName = {":/input/1.png"};

    setOutputsOnTop(false);
    setCanChangeSkin(true);
    setPixmap(m_pixmapSkinName[0]);
    setRotatable(false);
    setPortName("VCC");
    m_outputs.first()->setValue(true);
}

void InputVcc::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/input/1.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
