// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "or.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Or>();
}

Or::Or(QGraphicsItem *parent)
    : GraphicElement(ElementType::Or, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    qCDebug(zero) << "Creating or.";

    m_pixmapSkinName = QStringList{":/basic/or.png"};
    setPixmap(m_pixmapSkinName.first());

    setOutputsOnTop(true);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("OR");
    setToolTip(m_translatedName);
}

void Or::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_pixmapSkinName[0] = (defaultSkin) ? ":/basic/or.png" : fileName;
    setPixmap(m_pixmapSkinName[0]);
}
