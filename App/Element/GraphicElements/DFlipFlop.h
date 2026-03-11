// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "GraphicElement.h"
#include "ThemeManager.h"

class DFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    explicit DFlipFlop(QGraphicsItem *parent = nullptr);

    static QString pixmapPath()
    {
        return ":/Components/Memory/" + ThemeManager::themePath() + "/D-flipflop.svg";
    }

    void updatePortsProperties() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(DFlipFlop)

