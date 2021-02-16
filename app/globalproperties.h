/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
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
    static QByteArray settingToByteArray(const QList<int> &savedSettings);
    static QList<int> settingToIntList(const QByteArray &toSaveSettings);
};

#endif // GLOBALPROPERTIES_H