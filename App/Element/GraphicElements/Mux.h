// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "GraphicElement.h"

class Mux : public GraphicElement
{
    Q_OBJECT

public:
    explicit Mux(QGraphicsItem *parent = nullptr);

    void updatePortsProperties() override;
};

Q_DECLARE_METATYPE(Mux)
