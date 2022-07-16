/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"
#include "thememanager.h"

class SRFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    explicit SRFlipFlop(QGraphicsItem *parent = nullptr);

    static QString pixmapPath()
    {
        return ":/memory/" + ThemeManager::themePath() + "/SR-flipflop.svg";
    }

    void updatePortsProperties() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(SRFlipFlop)
