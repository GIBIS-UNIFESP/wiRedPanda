// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Cross-platform OS color scheme detection.
 *
 * Detects whether the operating system is in dark or light mode, without
 * depending on Qt 6.5+ QStyleHints::colorScheme().
 *
 * - Linux: D-Bus query to org.freedesktop.portal.Settings (freedesktop standard)
 * - Windows: Registry key HKCU\\...\\Personalize\\AppsUseLightTheme
 * - macOS: NSApp.effectiveAppearance
 * - Fallback: QPalette window-background lightness heuristic
 */

#pragma once

#include <QObject>

#ifdef HAVE_QTDBUS
class QDBusVariant;
#endif

/// Result of an OS color scheme query.
enum class ColorScheme { Light, Dark, Unknown };

/**
 * \class SystemThemeDetector
 * \brief Singleton that detects and monitors the OS color scheme.
 *
 * Call isDark() for a one-shot query.  Connect to colorSchemeChanged()
 * for live updates when the user toggles their OS theme.
 */
class SystemThemeDetector : public QObject
{
    Q_OBJECT

public:
    static SystemThemeDetector &instance();

    /// Returns the current OS color scheme.
    static ColorScheme colorScheme();

    /// Convenience: true when the OS is in dark mode.
    static bool isDark();

signals:
    void colorSchemeChanged(ColorScheme scheme);

private:
    explicit SystemThemeDetector(QObject *parent = nullptr);

    /// Platform-specific one-shot query.
    static ColorScheme queryPlatform();

    /// Sets up platform-specific change listeners.
    void setupListeners();

    /// Called by platform listeners when the scheme changes.
    void handleChange(ColorScheme newScheme);

    /// Catches QEvent::ApplicationPaletteChange for live theme monitoring.
    bool eventFilter(QObject *obj, QEvent *event) override;

#ifdef HAVE_QTDBUS
    /// D-Bus slot for org.freedesktop.portal.Settings.SettingChanged.
    Q_SLOT void onDbusSettingChanged(const QString &ns, const QString &key, const QDBusVariant &value);
#endif

    ColorScheme m_cached = ColorScheme::Unknown;
};
