// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xnor.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Xnor>();
}

Xnor::Xnor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xnor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_defaultSkins = QStringList{":/basic/xnor.png"};
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    updatePorts();
    setPortName("XNOR");
    setToolTip(m_translatedName);
}

void Xnor::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = (defaultSkin) ? ":/basic/xnor.png" : fileName;
    setPixmap(m_defaultSkins[0]);
}
