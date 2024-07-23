// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later7

#pragma once

#include "graphicelement.h"
#include "thememanager.h"

class JKFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    explicit JKFlipFlop(QGraphicsItem *parent = nullptr);

    static QString pixmapPath()
    {
        return ":/memory/" + ThemeManager::themePath() + "/JK-flipflop.svg";
    }

    void updatePortsProperties() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(JKFlipFlop)
