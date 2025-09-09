// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qneconnection.h"

class WirelessConnection : public QNEConnection
{
    Q_DECLARE_TR_FUNCTIONS(WirelessConnection)

public:
    enum { Type = QGraphicsItem::UserType + 6 };  // Next in sequence after PhysicalConnection(+5)
    int type() const override { return Type; }

    explicit WirelessConnection(QGraphicsItem *parent = nullptr);
    explicit WirelessConnection(const QString& label, QGraphicsItem *parent = nullptr);
    ~WirelessConnection() override = default;
    WirelessConnection(const WirelessConnection &other) : WirelessConnection(other.m_wirelessLabel, other.parentItem()) {}

    QString wirelessLabel() const { return m_wirelessLabel; }

protected:
    // Always invisible rendering
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    // No selection shape for wireless connections
    QPainterPath shape() const override;
    
    // Override scene events to prevent user interaction
    bool sceneEvent(QEvent *event) override;

private:
    void initializeWirelessConnection();
    
    QString m_wirelessLabel;
};

Q_DECLARE_METATYPE(WirelessConnection)