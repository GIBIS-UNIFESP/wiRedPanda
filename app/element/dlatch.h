/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class DLatch : public GraphicElement
{
    Q_OBJECT

public:
    explicit DLatch(QGraphicsItem *parent = nullptr);
    ~DLatch() override = default;

    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

