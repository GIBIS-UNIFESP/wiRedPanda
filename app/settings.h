// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSettings>

/*!
 * @class Settings
 * @brief Static utility class for managing the application settings
 *
 * The Settings class provides a static interface to the Qt settings system,
 * allowing application settings to be stored and retrieved in a consistent
 * manner. It also acts as a central point for accessing configuration values.
 */
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
    static QSettings *settingsInstance();
    static QSettings *settings;
};
