/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QString>

constexpr int globalClock = 10;

class GlobalProperties
{
public:
    inline static QString currentFile = {};
    inline static bool verbose = true;
    inline static const double version = QString(APP_VERSION).toDouble();
};
