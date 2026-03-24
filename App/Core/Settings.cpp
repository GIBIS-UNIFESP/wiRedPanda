// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Settings.h"

#include "App/Core/ThemeManager.h"

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

void Settings::setValue(const QString &key, const QVariant &value)
{
    settingsInstance()->setValue(key, value);
}

// --- Typed accessors ---

// Window state

QByteArray Settings::mainWindowGeometry()
{
    return value("MainWindow/geometry").toByteArray();
}

void Settings::setMainWindowGeometry(const QByteArray &geometry)
{
    setValue("MainWindow/geometry", geometry);
}

QByteArray Settings::mainWindowState()
{
    return value("MainWindow/windowState").toByteArray();
}

void Settings::setMainWindowState(const QByteArray &state)
{
    setValue("MainWindow/windowState", state);
}

QByteArray Settings::splitterGeometry()
{
    return value("MainWindow/splitter/geometry").toByteArray();
}

void Settings::setSplitterGeometry(const QByteArray &geometry)
{
    setValue("MainWindow/splitter/geometry", geometry);
}

QByteArray Settings::splitterState()
{
    return value("MainWindow/splitter/state").toByteArray();
}

void Settings::setSplitterState(const QByteArray &state)
{
    setValue("MainWindow/splitter/state", state);
}

QByteArray Settings::dolphinGeometry()
{
    return value("beWavedDolphin/geometry").toByteArray();
}

void Settings::setDolphinGeometry(const QByteArray &geometry)
{
    setValue("beWavedDolphin/geometry", geometry);
}

// UI preferences

bool Settings::fastMode()
{
    return value("fastMode").toBool();
}

void Settings::setFastMode(bool enabled)
{
    setValue("fastMode", enabled);
}

bool Settings::labelsUnderIcons()
{
    return value("labelsUnderIcons").toBool();
}

void Settings::setLabelsUnderIcons(bool enabled)
{
    setValue("labelsUnderIcons", enabled);
}

// Language

QString Settings::language()
{
    return value("language").toString();
}

void Settings::setLanguage(const QString &lang)
{
    setValue("language", lang);
}

// Theme

Theme Settings::theme()
{
    const int saved = value("theme").toInt();
    if (saved >= 0 && saved <= static_cast<int>(Theme::System)) {
        return static_cast<Theme>(saved);
    }
    return Theme::Light;
}

void Settings::setTheme(Theme theme)
{
    setValue("theme", static_cast<int>(theme));
}

// Recent files

QStringList Settings::recentFiles()
{
    return value("recentFileList").toStringList();
}

void Settings::setRecentFiles(const QStringList &files)
{
    setValue("recentFileList", files);
}

// Autosave

QStringList Settings::autosaveFiles()
{
    return value("autosaveFile").toStringList();
}

void Settings::setAutosaveFiles(const QStringList &files)
{
    setValue("autosaveFile", files);
}

bool Settings::hideV4Warning()
{
    return value("hideV4Warning").toBool();
}

void Settings::setHideV4Warning(bool hide)
{
    if (hide) {
        setValue("hideV4Warning", "true");
    } else {
        settingsInstance()->remove("hideV4Warning");
    }
}

QString Settings::updateCheckLastDate()
{
    return value("updateCheck/lastCheckDate").toString();
}

void Settings::setUpdateCheckLastDate(const QString &date)
{
    setValue("updateCheck/lastCheckDate", date);
}

QString Settings::updateCheckSkippedVersion()
{
    return value("updateCheck/skippedVersion").toString();
}

void Settings::setUpdateCheckSkippedVersion(const QString &version)
{
    setValue("updateCheck/skippedVersion", version);
}

