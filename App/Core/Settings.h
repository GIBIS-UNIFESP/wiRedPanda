// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Typed wrappers around QSettings for all application preferences.
 */

#pragma once

#include <QByteArray>
#include <QSettings>
#include <QStringList>

#include "App/Core/ThemeManager.h"

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

    // --- Typed accessors ---

    // Window state
    static QByteArray mainWindowGeometry();
    static void setMainWindowGeometry(const QByteArray &geometry);
    static QByteArray mainWindowState();
    static void setMainWindowState(const QByteArray &state);
    static QByteArray splitterGeometry();
    static void setSplitterGeometry(const QByteArray &geometry);
    static QByteArray splitterState();
    static void setSplitterState(const QByteArray &state);
    static QByteArray dolphinGeometry();
    static void setDolphinGeometry(const QByteArray &geometry);

    // UI preferences
    static bool fastMode();
    static void setFastMode(bool enabled);
    static bool labelsUnderIcons();
    static void setLabelsUnderIcons(bool enabled);
    static bool icPreviewDisabled();
    static void setIcPreviewDisabled(bool disabled);

    // Language
    static QString language();
    static void setLanguage(const QString &lang);

    // Theme
    static Theme theme();
    static void setTheme(Theme theme);

    // Recent files
    static QStringList recentFiles();
    static void setRecentFiles(const QStringList &files);

    // Autosave
    static QStringList autosaveFiles();
    static void setAutosaveFiles(const QStringList &files);

    // V4 format warning suppression
    static bool hideV4Warning();
    static void setHideV4Warning(bool hide);

    // Update checker
    static QString updateCheckLastDate();
    static void setUpdateCheckLastDate(const QString &date);
    static QString updateCheckSkippedVersion();
    static void setUpdateCheckSkippedVersion(const QString &version);
    /// Global opt-out of update checks (for offline/managed installs); default false (enabled).
    static bool updateChecksDisabled();
    static void setUpdateChecksDisabled(bool disabled);

    // Exercise progress
    static QStringList completedExercises();
    static void        setCompletedExercises(const QStringList &ids);
    static int         exerciseProgress(const QString &exerciseId); ///< Returns -1 if no record.
    static void        setExerciseProgress(const QString &exerciseId, int step);

    // Tour progress
    static QStringList completedTours();
    static void        setCompletedTours(const QStringList &ids);
    static int         tourProgress(const QString &tourId); ///< Returns -1 if no record.
    static void        setTourProgress(const QString &tourId, int step);

    /// Whether the built-in welcome tour has already been auto-started once. Set the first
    /// time MainWindow::show() runs, regardless of whether the tour is completed or dismissed,
    /// so it never auto-starts again after that first launch.
    static bool welcomeTourShown();
    static void setWelcomeTourShown(bool shown);

    /// Corner of the workspace where the minimap overview is anchored.
    enum class MinimapCorner {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
    };

    // Minimap preferences
    static bool minimapVisible();
    static void setMinimapVisible(bool visible);
    static MinimapCorner minimapCorner();
    static void setMinimapCorner(MinimapCorner corner);

private:
    static QVariant value(const QString &key);
    static void setValue(const QString &key, const QVariant &value);
    static QSettings *settingsInstance();

    // --- Members ---

    inline static QSettings *settings = nullptr;
};
