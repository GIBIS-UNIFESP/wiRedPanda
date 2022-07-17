// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class GlobalProperties
{
public:
    inline static QString currentFile = {};
    inline static bool skipInit = false;
    inline static bool verbose = true;
    inline static const double version = QString(APP_VERSION).toDouble();
    inline static const int gridSize = 16;
};
