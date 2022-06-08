/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QString>

class GlobalProperties
{
public:
    inline const static int globalClock = 10;
    inline static QString currentFile = {};
    inline static bool skipInit = false;
    inline static bool verbose = true;
    inline static const double version = QString(APP_VERSION).toDouble();
};
