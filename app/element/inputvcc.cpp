// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputvcc.h"

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::VCC, ElementGroup::STATICINPUT, 0, 0, 1, 1, parent)
{
    pixmapSkinName = {":/input/1.png"};

    setOutputsOnTop(false);
    setCanChangeSkin(true);
    setPixmap(pixmapSkinName[0]);
    setRotatable(false);
    setPortName("VCC");
    m_outputs.first()->setValue(true);
}

void InputVcc::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/input/1.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
