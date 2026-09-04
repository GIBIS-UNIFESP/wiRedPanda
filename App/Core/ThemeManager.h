// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Theme management types and singleton ThemeManager.
 */

#pragma once

#include <functional>

#include <QColor>
#include <QObject>
#include <QPalette>

/// Enumeration of available application themes.
enum class Theme { Light, Dark, System };

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

    // --- Members: Minimap Colors ---

    /// Flat fill color for CanvasItem::paintElementsSimplifiedInto()'s per-element rects -- the
    /// large-circuit minimap thumbnail fallback that skips each element's real appearance/label
    /// (see that method's own doc comment for why).
    QColor m_minimapElementBrush;

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

    // --- Members: Port Pen Colors ---

    QColor m_portUnknownPen;
    QColor m_portInactivePen;
    QColor m_portActivePen;
    QColor m_portErrorPen;

    // --- Members: Port Hover ---

    QColor m_portHoverPort;

    // --- Members: Port Hover Label ---

    QColor m_portHoverLabelBg;
    QColor m_portHoverLabelText;
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

    /// Returns the current ThemeAttributes color set.
    static const ThemeAttributes &attributes();

    // --- Theme Management ---

    /// Switches the application to \a theme and emits themeChanged().
    static void setTheme(const Theme theme);

    /// Returns the effective theme (Light or Dark), resolving System to the OS preference.
    static Theme effectiveTheme();

    /// Hook a Widgets-only host uses to apply a QApplication-level QPalette/style sheet
    /// whenever the effective theme changes -- this class has no notion of QApplication/
    /// QPalette-as-applied-state itself (ThemeAttributes only holds resolved *color values*,
    /// consumed directly by the domain-layer paint() methods). Defaults to a no-op, so a
    /// non-Widgets host (or a unit test) simply doesn't get any app-wide styling applied.
    using WidgetsStylingApplier = std::function<void(Theme effectiveTheme)>;
    static void setWidgetsStylingApplier(WidgetsStylingApplier applier);

signals:
    // --- Signals ---

    /// Emitted whenever the active theme changes.
    void themeChanged();

private:
    // --- Lifecycle ---

    explicit ThemeManager(QObject *parent = nullptr);

    /// Resolves Theme::System to Light or Dark based on the OS color scheme.
    /// Qt 6.5+: reads QStyleHints::colorScheme(). Qt < 6.5: palette lightness heuristic.
    static Theme resolveSystemTheme();

    /// Called when the OS color scheme changes (connected to QStyleHints::colorSchemeChanged on Qt 6.5+).
    void onSystemColorSchemeChanged();

    // --- Members ---

    Theme m_theme = Theme::System;
    ThemeAttributes m_attributes;

    static WidgetsStylingApplier s_widgetsStylingApplier;
};
