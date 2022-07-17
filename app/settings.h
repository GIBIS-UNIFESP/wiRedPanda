// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSettings>

class Settings
{
public:
    Settings() = delete;

    static QString fileName();
    static QVariant value(const QString &key);
    static bool contains(const QString &key);
    static void remove(const QString &key);
    static void setValue(const QString &key, const QVariant &value);

private:
    inline static QSettings *const settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GIBIS-UNIFESP", "WiRedPanda");
};
