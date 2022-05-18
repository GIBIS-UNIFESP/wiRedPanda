// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jklatch.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<JKLatch>();
}

JKLatch::JKLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::JKLatch, ElementGroup::Memory, 2, 2, 2, 2, parent)
{
    qCDebug(zero) << "Creating jklatch.";

    m_pixmapSkinName = QStringList{":/memory/JK-latch.png"};
    setPixmap(m_pixmapSkinName.first());

    setRotatable(false);
    setCanChangeSkin(true);
    JKLatch::updatePorts();
    setCanChangeSkin(true);
    setPortName("JK Latch");
    setToolTip(m_translatedName);
}

void JKLatch::updatePorts()
{
    input(0)->setPos(topPosition(), 13);
    input(1)->setPos(topPosition(), 45);

    output(0)->setPos(bottomPosition(), 15);
    output(1)->setPos(bottomPosition(), 45);
}
