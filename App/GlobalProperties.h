// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QVersionNumber>

#define VERSION(string) QVersionNumber::fromString(string).normalized()

class GlobalProperties
{
public:
    // --- Runtime State ---

    inline static QString currentDir = {};
    inline static bool skipInit = false;
    inline static bool interactiveMode = true;

    // --- Application Constants ---

    /// Application version number, initialized from the APP_VERSION CMake definition.
    inline static const QVersionNumber version = VERSION(APP_VERSION);
    inline static const int gridSize = 16;
    inline static const int maxRecentFiles = 10;
};
