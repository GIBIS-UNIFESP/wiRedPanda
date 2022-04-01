// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "or.h"

namespace {
int id = qRegisterMetaType<Or>();
}

Or::Or(QGraphicsItem *parent)
    : GraphicElement(ElementType::Or, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/or.png"};
    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("OR");
    setToolTip(m_translatedName);
}

void Or::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/or.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
