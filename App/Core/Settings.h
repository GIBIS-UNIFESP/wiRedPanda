// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Typed wrappers around QSettings for all application preferences.
 */

#pragma once

#include <QSettings>

/**
 * \class Settings
 * \brief Static utility class for managing the application settings.
 *
 * The Settings class provides a static interface to the Qt settings system,
 * allowing application settings to be stored and retrieved in a consistent
 * manner. It also acts as a central point for accessing configuration values.
 */
class Settings
{
public:
    Settings() = delete;

    /// Returns the path to the settings file on disk.
    static QString fileName();

    // --- Accessors ---

    /// Returns the value stored under \a key, or an invalid QVariant if not set.
    static QVariant value(const QString &key);
    /// Returns \c true if a setting exists for \a key.
    static bool contains(const QString &key);
    /// Removes the setting identified by \a key.
    static void remove(const QString &key);
    /// Sets the setting identified by \a key to \a value.
    static void setValue(const QString &key, const QVariant &value);

private:
    // --- Internal helpers ---

    static QSettings *settingsInstance();

    // --- Members ---

    inline static QSettings *settings = nullptr;
};

