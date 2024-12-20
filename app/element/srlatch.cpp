// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "srlatch.h"

#include "globalproperties.h"
#include "qneport.h"

namespace
{
    int id = qRegisterMetaType<SRLatch>();
}

SRLatch::SRLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::SRLatch, ElementGroup::Memory, pixmapPath(), tr("SR-LATCH"), tr("SR-Latch"), 2, 2, 2, 2, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);

    SRLatch::updatePortsProperties();
}

void SRLatch::updatePortsProperties()
{
    inputPort(0)->setPos(0, 16);      inputPort(0)->setName("Set");
    inputPort(1)->setPos(0, 48);      inputPort(1)->setName("Reset");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void SRLatch::updateTheme()
{
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
