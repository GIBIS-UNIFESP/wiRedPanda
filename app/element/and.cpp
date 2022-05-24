// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "and.h"

#include "common.h"
#include "elementfactory.h"

namespace
{
int id = qRegisterMetaType<And>();
}

And::And(QGraphicsItem *parent)
    : GraphicElement(ElementType::And, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_defaultSkins = QStringList{":/basic/and.png"};
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    setOutputsOnTop(true);
    setPortName("AND");
    setToolTip(m_translatedName);
    updatePorts();
}

void And::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins.first() = (defaultSkin) ? m_defaultSkins.first() : fileName;
    setPixmap(m_defaultSkins.first());
}
