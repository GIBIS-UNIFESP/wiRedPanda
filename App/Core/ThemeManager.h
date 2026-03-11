// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QObject>
#include <QPalette>

#include "App/Core/Application.h"

enum class Theme { Light, Dark };

/*!
 * \class ThemeAttributes
 * \brief Contains all color attributes for a theme
 *
 * This class holds all the color-related attributes used throughout the
 * application UI, including colors for the scene background, selection
 * indicators, connections, ports, and other UI elements.
 */
class ThemeAttributes
{
public:
    // --- Theme Setup ---

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

    QColor m_connectionInvalid = QColor(Qt::red);
    QColor m_connectionInactive;
    QColor m_connectionActive;
    QColor m_connectionSelected;

    // --- Members: Port Brush Colors ---

    QColor m_portInvalidBrush;
    QColor m_portInactiveBrush;
    QColor m_portActiveBrush;
    QColor m_portOutputBrush;

    // --- Members: Port Pen Colors ---

    QColor m_portInvalidPen;
    QColor m_portInactivePen;
    QColor m_portActivePen;
    QColor m_portOutputPen;

    // --- Members: Port Hover ---

    QColor m_portHoverPort;

private:
    QPalette const m_defaultPalette = qApp->palette();
};

/*!
 * @class ThemeManager
 * @brief Singleton class for managing application themes
 *
 * ThemeManager provides a central point for accessing and changing the
 * application theme. It maintains the current theme setting and notifies
 * the application when the theme changes.
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    // --- Singleton Access ---

    static ThemeManager &instance()
    {
        static ThemeManager instance;
        return instance;
    }

    // --- Queries ---

    static QString themePath();
    static Theme theme();
    static ThemeAttributes attributes();

    // --- Theme Management ---

    static void setTheme(const Theme theme);

signals:
    // --- Signals ---

    void themeChanged();

private:
    // --- Lifecycle ---

    explicit ThemeManager(QObject *parent = nullptr);

    // --- Members ---

    Theme m_theme = Theme::Light;
    ThemeAttributes m_attributes;
};

