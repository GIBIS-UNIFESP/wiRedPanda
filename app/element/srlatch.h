// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"
#include "thememanager.h"

class SRLatch : public GraphicElement
{
    Q_OBJECT

public:
    explicit SRLatch(QGraphicsItem *parent = nullptr);

    static QString pixmapPath()
    {
        return ":/memory/" + ThemeManager::themePath() + "/SR-latch.svg";
    }

    void updatePortsProperties() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(SRLatch)
