/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QString>

constexpr auto globalClock = 10;

class GlobalProperties
{
public:
    static QString currentFile;
    static bool soundEnabled;
    static double toDouble(const QString &txtVersion, bool *ok);
    static double version;
};
