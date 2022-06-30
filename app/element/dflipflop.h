/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

    void updatePorts() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(DFlipFlop)
