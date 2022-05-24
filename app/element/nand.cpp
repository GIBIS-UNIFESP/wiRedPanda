// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nand.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Nand>();
}

Nand::Nand(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nand, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_defaultSkins = QStringList{":/basic/nand.png"};
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    updatePorts();
    setCanChangeSkin(true);
    setPortName("NAND");
    setToolTip(m_translatedName);
}

void Nand::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = (defaultSkin) ? ":/basic/nand.png" : fileName;
    setPixmap(m_defaultSkins[0]);
}
