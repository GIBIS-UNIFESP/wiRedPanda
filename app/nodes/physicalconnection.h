// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qneconnection.h"

class PhysicalConnection : public QNEConnection
{
    Q_DECLARE_TR_FUNCTIONS(PhysicalConnection)

public:
    enum { Type = QGraphicsItem::UserType + 10 };
    int type() const override { return Type; }

    explicit PhysicalConnection(QGraphicsItem *parent = nullptr);
    ~PhysicalConnection() override = default;
    PhysicalConnection(const PhysicalConnection &other) : PhysicalConnection(other.parentItem()) {}

    // Uses base class paint() - normal visible rendering
    // Uses base class shape() - normal selection behavior
    // Uses base class sceneEvent() - normal user interaction
};

Q_DECLARE_METATYPE(PhysicalConnection)