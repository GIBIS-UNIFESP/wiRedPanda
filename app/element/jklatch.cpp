// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jklatch.h"

JKLatch::JKLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::JKLATCH, ElementGroup::MEMORY, 2, 2, 2, 2, parent)
{
    setPixmap(":/memory/JK-latch.png");
    setRotatable(false);
    setCanChangeSkin(true);
    updatePorts();
    setCanChangeSkin(true);
    setPortName("JK Latch");
}

void JKLatch::updatePorts()
{
    input(0)->setPos(topPosition(), 13);
    input(1)->setPos(topPosition(), 45);

    output(0)->setPos(bottomPosition(), 15);
    output(1)->setPos(bottomPosition(), 45);
}
