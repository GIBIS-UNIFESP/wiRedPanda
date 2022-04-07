// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xnor.h"

namespace {
int id = qRegisterMetaType<Xnor>();
}

Xnor::Xnor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xnor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/xnor.png"};

    setOutputsOnTop(true);
    setCanChangeSkin(true);
    setPixmap(m_pixmapSkinName[0]);
    updatePorts();
    setPortName("XNOR");
    setToolTip(m_translatedName);
}

void Xnor::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/xnor.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
