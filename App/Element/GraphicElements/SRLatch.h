// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "GraphicElement.h"
#include "ThemeManager.h"

class SRLatch : public GraphicElement
{
    Q_OBJECT

public:
    explicit SRLatch(QGraphicsItem *parent = nullptr);

    static QString pixmapPath()
    {
        return ":/Components/Memory/" + ThemeManager::themePath() + "/SR-latch.svg";
    }

    void updatePortsProperties() override;
    void updateTheme() override;
};

Q_DECLARE_METATYPE(SRLatch)
