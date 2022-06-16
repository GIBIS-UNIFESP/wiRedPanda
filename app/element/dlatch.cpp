// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlatch.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<DLatch>();
}

DLatch::DLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::DLatch, ElementGroup::Memory, 2, 2, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setRotatable(false);
    setCanChangeSkin(true);
    DLatch::updatePorts();
    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    inputPort(0)->setName("Data");
    inputPort(1)->setName("Enable");

    outputPort(0)->setName("Q");
    outputPort(1)->setName("~Q");

    outputPort(0)->setDefaultValue(Status::Inactive);
    outputPort(1)->setDefaultValue(Status::Active);
}

void DLatch::updatePorts()
{
    inputPort(0)->setPos(leftPosition(), 13);   /* Data   */
    inputPort(1)->setPos(leftPosition(), 45);   /* Enable */

    outputPort(0)->setPos(rightPosition(), 15); /* Q      */
    outputPort(1)->setPos(rightPosition(), 45); /* ~Q     */
}

void DLatch::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
