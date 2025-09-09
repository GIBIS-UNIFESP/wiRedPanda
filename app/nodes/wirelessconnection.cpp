// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wirelessconnection.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QEvent>

WirelessConnection::WirelessConnection(QGraphicsItem *parent)
    : QNEConnection(parent)
{
    // Default constructor for Qt meta type system
    initializeWirelessConnection();
}

WirelessConnection::WirelessConnection(const QString& label, QGraphicsItem *parent)
    : QNEConnection(parent), m_wirelessLabel(label)
{
    initializeWirelessConnection();
}

void WirelessConnection::initializeWirelessConnection()
{
    // Wireless connections are auto-managed and should not be user-selectable
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    
    // Set lower Z-value to ensure they don't interfere with user interactions
    setZValue(-10);
}

void WirelessConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)  
    Q_UNUSED(widget)
    
    // Wireless connections are completely invisible
    return;
}

QPainterPath WirelessConnection::shape() const
{
    // Return empty path - no selection shape for wireless connections
    return QPainterPath();
}

bool WirelessConnection::sceneEvent(QEvent *event)
{
    Q_UNUSED(event)
    
    // Ignore all scene events - wireless connections are non-interactive
    return false;
}