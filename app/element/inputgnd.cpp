// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputgnd.h"

InputGnd::InputGnd(QGraphicsItem *parent)
    : GraphicElement(ElementType::GND, ElementGroup::STATICINPUT, 0, 0, 1, 1, parent)
{
    pixmapSkinName = {":/input/0.png"};

    setOutputsOnTop(false);
    setCanChangeSkin(true);
    setPixmap(pixmapSkinName[0]);
    setRotatable(false);
    setPortName("GND");
    m_outputs.first()->setValue(false);
}

void InputGnd::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/input/0.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
