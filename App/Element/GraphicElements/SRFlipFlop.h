// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "GraphicElement.h"
#include "ThemeManager.h"

class SRFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    explicit SRFlipFlop(QGraphicsItem *parent = nullptr);

    static QString pixmapPath()
    {
        return ":/Components/Memory/" + ThemeManager::themePath() + "/SR-flipflop.svg";
    }

    void updatePortsProperties() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(SRFlipFlop)
