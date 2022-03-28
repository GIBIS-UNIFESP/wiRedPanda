/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QApplication>
#include <QColor>
#include <QObject>
#include <QPalette>

class Editor;

enum class Theme { Light, Dark };

class ThemeManager;

class ThemeAttrs
{
public:
    ThemeAttrs();

    QColor scene_bgBrush;
    QColor scene_bgDots;

    QColor selectionBrush;
    QColor selectionPen;

    QColor qneConnection_true;
    QColor qneConnection_false;
    QColor qneConnection_invalid;
    QColor qneConnection_selected;

    QColor graphicElement_labelColor;

    QColor qnePort_hoverPort;
    QColor qnePort_true_pen;
    QColor qnePort_true_brush;
    QColor qnePort_false_pen;
    QColor qnePort_false_brush;
    QColor qnePort_invalid_pen;
    QColor qnePort_invalid_brush;
    QColor qnePort_output_pen;
    QColor qnePort_output_brush;

    void setTheme(Theme theme);

private:
    QPalette const defaultPalette = qApp->palette();
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager *globalManager;

    explicit ThemeManager(QObject *parent = nullptr);

    QString currentTheme() const;
    void setCurrentTheme(const Theme &theme);

    Theme theme() const;
    void setTheme(Theme theme);

    void initialize();
    ThemeAttrs getAttrs() const;

signals:
    void themeChanged();

private:
    Theme m_theme;
    ThemeAttrs m_attrs;
};

