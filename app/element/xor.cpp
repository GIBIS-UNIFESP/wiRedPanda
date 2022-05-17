// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xor.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Xor>();
}

Xor::Xor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    qCDebug(zero) << "Creating xor.";

    m_pixmapSkinName.append(":/basic/xor.png");
    setPixmap(m_pixmapSkinName.first());

    setOutputsOnTop(true);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("XOR");
    setToolTip(m_translatedName);
}

void Xor::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_pixmapSkinName[0] = (defaultSkin) ? ":/basic/xor.png" : fileName;
    setPixmap(m_pixmapSkinName[0]);
}
