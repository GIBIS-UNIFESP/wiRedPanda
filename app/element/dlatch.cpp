
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlatch.h"

#include "globalproperties.h"
#include "qneport.h"

namespace
{
    int id = qRegisterMetaType<DLatch>();
}

DLatch::DLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::DLatch, ElementGroup::Memory, pixmapPath(), tr("D-LATCH"), tr("D-Latch"), 2, 2, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    DLatch::updatePortsProperties();
}

void DLatch::updatePortsProperties()
{
    inputPort(0)->setPos(0, 16);      inputPort(0)->setName("Data");
    inputPort(1)->setPos(0, 48);      inputPort(1)->setName("Enable");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void DLatch::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
