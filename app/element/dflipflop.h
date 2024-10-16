// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"
#include "thememanager.h"

class DFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    explicit DFlipFlop(QGraphicsItem *parent = nullptr);

    static QString pixmapPath()
    {
        return ":/memory/" + ThemeManager::themePath() + "/D-flipflop.svg";
    }

    void updatePortsProperties() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(DFlipFlop)
