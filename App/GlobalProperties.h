// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class GlobalProperties
{
public:
    // --- Runtime State ---

    inline static QString currentDir = {};
    inline static bool interactiveMode = true;

    // --- Application Constants ---

    inline static const int gridSize = 16;
    inline static const int maxRecentFiles = 10;
};
