// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QVersionNumber>

#define VERSION(string) QVersionNumber::fromString(string)

class GlobalProperties
{
public:
    inline static QString currentDir = {};
    inline static bool skipInit = false;
    inline static bool verbose = true;
    inline static const QVersionNumber version = VERSION(QString(APP_VERSION));
    inline static const int gridSize = 16;
    inline static const int maxRecentFiles = 10;
};
