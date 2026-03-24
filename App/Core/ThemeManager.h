// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Theme management types and singleton ThemeManager.
 */

#pragma once

#include <QColor>
#include <QObject>

/// Enumeration of available application themes.
enum class Theme { Light, Dark };

/**
 * \class ThemeAttributes
 * \brief Contains all color attributes for a theme.
 *
 * This class holds all the color-related attributes used throughout the
 * application UI, including colors for the scene background, selection
 * indicators, connections, ports, and other UI elements.
 */
class ThemeAttributes
{
public:
    // --- Theme Setup ---

    /// Applies \a theme colors to all color attributes in this object.
    void setTheme(const Theme theme);

    // --- Members: Scene Colors ---

    QColor m_sceneBgBrush;
    QColor m_sceneBgDots;

    // --- Members: Selection Colors ---

    QColor m_selectionBrush;
    QColor m_selectionPen;

    // --- Members: Label Colors ---

    QColor m_graphicElementLabelColor;

    // --- Members: Connection Colors ---

    QColor m_connectionUnknown;
    QColor m_connectionInactive;
    QColor m_connectionActive;
    QColor m_connectionError = QColor(Qt::red);
    QColor m_connectionSelected;

    // --- Members: Port Brush Colors ---

    QColor m_portUnknownBrush;
    QColor m_portInactiveBrush;
    QColor m_portActiveBrush;
    QColor m_portErrorBrush;
    QColor m_portOutputBrush;

    // --- Members: Port Pen Colors ---

    QColor m_portUnknownPen;
    QColor m_portInactivePen;
    QColor m_portActivePen;
    QColor m_portErrorPen;
    QColor m_portOutputPen;

    // --- Members: Port Hover ---

    QColor m_portHoverPort;
};

/**
 * \class ThemeManager
 * \brief Singleton class for managing application themes.
 *
 * ThemeManager provides a central point for accessing and changing the
 * application theme. It maintains the current theme setting and notifies
 * the application when the theme changes.
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    /// Returns the singleton ThemeManager instance.
    static ThemeManager &instance()
    {
        static ThemeManager instance;
        return instance;
    }

    /// Returns the resource path prefix for the current theme (e.g. "dark" or "light").
    static QString themePath();

    /// Returns the currently active theme.
    static Theme theme();

    /// Returns a copy of the current ThemeAttributes color set.
    static ThemeAttributes attributes();

    // --- Theme Management ---

    /// Switches the application to \a theme and emits themeChanged().
    static void setTheme(const Theme theme);

signals:
    // --- Signals ---

    /// Emitted whenever the active theme changes.
    void themeChanged();

private:
    // --- Lifecycle ---

    explicit ThemeManager(QObject *parent = nullptr);

    // --- Members ---

    Theme m_theme = Theme::Light;
    ThemeAttributes m_attributes;
};

