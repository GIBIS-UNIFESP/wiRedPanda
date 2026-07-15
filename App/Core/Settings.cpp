// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Settings.h"

#include "App/Core/ThemeManager.h"

QSettings *Settings::settingsInstance()
{
    if (!Settings::settings) {
#ifdef Q_OS_WASM
        // On WebAssembly, NativeFormat maps directly to browser localStorage,
        // providing persistent storage across sessions without filesystem sync.
        Settings::settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "GIBIS-UNIFESP", "wiRedPanda");
#else
        // IniFormat produces a human-readable text file rather than the platform registry
        // (Windows) or plist (macOS), making it easy to inspect and version-control settings.
        // UserScope stores the file in the user's home/config directory so that multiple
        // system users don't share the same preference file.
        Settings::settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GIBIS-UNIFESP", "wiRedPanda");
#endif
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
    // Default to labels-visible: the beginner audience benefits from readable toolbar
    // buttons. Power users can turn labels off via View → Labels under icons.
    const QVariant saved = value("labelsUnderIcons");
    return saved.isValid() ? saved.toBool() : true;
}

void Settings::setLabelsUnderIcons(bool enabled)
{
    setValue("labelsUnderIcons", enabled);
}

bool Settings::icPreviewDisabled()
{
    return value("icPreviewDisabled").toBool();
}

void Settings::setIcPreviewDisabled(bool disabled)
{
    setValue("icPreviewDisabled", disabled);
}

bool Settings::updateChecksDisabled()
{
    return value("updateChecksDisabled").toBool();
}

void Settings::setUpdateChecksDisabled(bool disabled)
{
    setValue("updateChecksDisabled", disabled);
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
    const QVariant v = value("theme");
    if (!v.isValid()) {
        return Theme::System;
    }
    const int saved = v.toInt();
    if (saved >= 0 && saved <= static_cast<int>(Theme::System)) {
        return static_cast<Theme>(saved);
    }
    return Theme::System;
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

// Exercise progress

QStringList Settings::completedExercises()
{
    return value("exercises/completed").toStringList();
}

void Settings::setCompletedExercises(const QStringList &ids)
{
    setValue("exercises/completed", ids);
}

int Settings::exerciseProgress(const QString &exerciseId)
{
    const QVariant v = value(QStringLiteral("exercises/progress/") + exerciseId);
    return v.isValid() ? v.toInt() : -1;
}

void Settings::setExerciseProgress(const QString &exerciseId, int step)
{
    setValue(QStringLiteral("exercises/progress/") + exerciseId, step);
}

// Tour progress

QStringList Settings::completedTours()
{
    return value("tours/completed").toStringList();
}

void Settings::setCompletedTours(const QStringList &ids)
{
    setValue("tours/completed", ids);
}

int Settings::tourProgress(const QString &tourId)
{
    const QVariant v = value(QStringLiteral("tours/progress/") + tourId);
    return v.isValid() ? v.toInt() : -1;
}

void Settings::setTourProgress(const QString &tourId, int step)
{
    setValue(QStringLiteral("tours/progress/") + tourId, step);
}

bool Settings::welcomeTourShown()
{
    return value("tours/welcomeShown").toBool();
}

void Settings::setWelcomeTourShown(bool shown)
{
    setValue("tours/welcomeShown", shown);
}

// Minimap preferences
bool Settings::minimapVisible()
{
    const QVariant v = value("minimap/visible");
    if (!v.isValid())
        return true; // default visible
    return v.toBool();
}

void Settings::setMinimapVisible(bool visible)
{
    setValue("minimap/visible", visible);
}

QRect Settings::minimapGeometry()
{
    return value("minimap/geometry").toRect();
}

void Settings::setMinimapGeometry(const QRect &geometry)
{
    setValue("minimap/geometry", geometry);
}
