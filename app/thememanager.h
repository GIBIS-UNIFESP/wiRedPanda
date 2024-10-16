
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "application.h"

#include <QColor>
#include <QObject>
#include <QPalette>

enum class Theme { Light, Dark };

class ThemeAttributes
{
public:
    void setTheme(const Theme theme);

    QColor m_sceneBgBrush;
    QColor m_sceneBgDots;

    QColor m_selectionBrush;
    QColor m_selectionPen;

    QColor m_graphicElementLabelColor;

    QColor m_connectionInvalid = QColor(Qt::red);
    QColor m_connectionInactive;
    QColor m_connectionActive;
    QColor m_connectionSelected;

    QColor m_portInvalidBrush;
    QColor m_portInactiveBrush;
    QColor m_portActiveBrush;
    QColor m_portOutputBrush;

    QColor m_portInvalidPen;
    QColor m_portInactivePen;
    QColor m_portActivePen;
    QColor m_portOutputPen;

    QColor m_portHoverPort;

private:
    QPalette const m_defaultPalette = qApp->palette();
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager &instance()
    {
        static ThemeManager instance;
        return instance;
    }

    static QString themePath();
    static Theme theme();
    static ThemeAttributes attributes();
    static void setTheme(const Theme theme);

signals:
    void themeChanged();

private:
    explicit ThemeManager(QObject *parent = nullptr);

    Theme m_theme = Theme::Light;
    ThemeAttributes m_attributes;
};
