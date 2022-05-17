/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

    QColor m_qneConnectionFalse;
    QColor m_qneConnectionInvalid;
    QColor m_qneConnectionSelected;
    QColor m_qneConnectionTrue;

    QColor m_qnePortFalseBrush;
    QColor m_qnePortInvalidBrush;
    QColor m_qnePortOutputBrush;
    QColor m_qnePortTrueBrush;

    QColor m_qnePortFalsePen;
    QColor m_qnePortInvalidPen;
    QColor m_qnePortOutputPen;
    QColor m_qnePortTruePen;

    QColor m_qnePortHoverPort;

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

