// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlatch.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<DLatch>();
}

DLatch::DLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::DLatch, ElementGroup::Memory, 2, 2, 2, 2, parent)
{
    m_defaultSkins = QStringList{":/memory/D-latch.png"};
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setRotatable(false);
    setCanChangeSkin(true);
    DLatch::updatePorts();
    setPortName("D Latch");
    setToolTip(m_translatedName);

    input(0)->setName("Data");
    input(1)->setName("Enable");

    output(0)->setName("Q");
    output(1)->setName("~Q");

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);
}

void DLatch::updatePorts()
{
    input(0)->setPos(leftPosition(), 13);   /* Data   */
    input(1)->setPos(leftPosition(), 45);   /* Enable */

    output(0)->setPos(rightPosition(), 15); /* Q      */
    output(1)->setPos(rightPosition(), 45); /* ~Q     */
}
