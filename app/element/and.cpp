// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "and.h"
#include "elementfactory.h"

namespace {
int id = qRegisterMetaType<And>();
}

And::And(QGraphicsItem *parent)
    : GraphicElement(ElementType::And, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_pixmapSkinName = {":/basic/and.png"};

    setOutputsOnTop(true);
    setPixmap(m_pixmapSkinName[0]);
    updatePorts();
    setCanChangeSkin(true);
    setPortName("AND");
    setToolTip(m_translatedName);
}

void And::setSkin(bool defaultSkin, const QString &filename)
{
    m_pixmapSkinName[0] = defaultSkin ? ":/basic/and.png" : filename;
    setPixmap(m_pixmapSkinName[0]);
}
