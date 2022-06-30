/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

#include <QObject>

class Mux : public GraphicElement
{
    Q_OBJECT

public:
    explicit Mux(QGraphicsItem *parent = nullptr);

    void updatePorts() override;
};

Q_DECLARE_METATYPE(Mux)
