// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Settings.h"

QSettings *Settings::settingsInstance()
{
    if (!Settings::settings) {
        // IniFormat produces a human-readable text file rather than the platform registry
        // (Windows) or plist (macOS), making it easy to inspect and version-control settings.
        // UserScope stores the file in the user's home/config directory so that multiple
        // system users don't share the same preference file.
        Settings::settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GIBIS-UNIFESP", "wiRedPanda");
    }
    return Settings::settings;
}

QString Settings::fileName()
{
    return settingsInstance()->fileName();
}

QVariant Settings::value(const QString &key)
{
    return settingsInstance()->value(key);
}

bool Settings::contains(const QString &key)
{
    return settingsInstance()->contains(key);
}

void Settings::remove(const QString &key)
{
    settingsInstance()->remove(key);
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    settingsInstance()->setValue(key, value);
}

