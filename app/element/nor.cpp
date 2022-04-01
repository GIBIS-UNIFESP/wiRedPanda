// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nor.h"

namespace {
int id = qRegisterMetaType<Nor>();
}

Nor::Nor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/nor.png"};
    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("NOR");
    setToolTip(m_translatedName);
}

void Nor::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/nor.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
