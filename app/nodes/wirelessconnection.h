// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qneconnection.h"

class WirelessConnection : public QNEConnection
{
    Q_DECLARE_TR_FUNCTIONS(WirelessConnection)

public:
    explicit WirelessConnection(QGraphicsItem *parent = nullptr);
    ~WirelessConnection() override = default;

protected:
    // Invisible rendering - only essential override needed
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

Q_DECLARE_METATYPE(WirelessConnection)
