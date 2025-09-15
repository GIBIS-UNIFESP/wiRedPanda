// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wirelessconnection.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

WirelessConnection::WirelessConnection(QGraphicsItem *parent)
    : QNEConnection(parent)
{
    // Configure for wireless use: invisible, non-interactive, background layer
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setZValue(-10);
}

void WirelessConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
    // Wireless connections are invisible - no rendering needed
}
