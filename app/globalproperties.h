/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GLOBALPROPERTIES_H
#define GLOBALPROPERTIES_H

#include <QString>

#define GLOBALCLK 10

class GlobalProperties
{
public:
    static QString currentFile;
    static double version;
    static bool soundEnabled;

    static double toDouble(const QString &txtVersion, bool *ok);
};

#endif // GLOBALPROPERTIES_H