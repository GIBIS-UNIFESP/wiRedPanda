// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nor.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Nor>();
}

Nor::Nor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_defaultSkins = QStringList{":/basic/nor.png"};
    setPixmap(m_defaultSkins.first());

    setOutputsOnTop(true);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("NOR");
    setToolTip(m_translatedName);
}

void Nor::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = (defaultSkin) ? ":/basic/nor.png" : fileName;
    setPixmap(m_defaultSkins[0]);
}
