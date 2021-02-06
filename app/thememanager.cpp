// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thememanager.h"

#include <QApplication>
#include <QSettings>

ThemeManager *ThemeManager::globalMngr = nullptr;

Theme ThemeManager::theme() const
{
    return m_theme;
}

void ThemeManager::setTheme(const Theme &theme)
{
    m_attrs.setTheme(theme);
    if (m_theme != theme) {
        m_theme = theme;
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
        settings.setValue("theme", static_cast<int>(theme));
        emit themeChanged();
    }
}

void ThemeManager::initialize()
{
    emit themeChanged();
}

ThemeAttrs ThemeManager::getAttrs() const
{
    return m_attrs;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    m_theme = Theme::Panda_Light;
    if (settings.contains("theme")) {
        setTheme(static_cast<Theme>(settings.value("theme").toInt()));
    } else {
        setTheme(Theme::Panda_Light);
    }
}

ThemeAttrs::ThemeAttrs()
{
    setTheme(Theme::Panda_Light);
}

void ThemeAttrs::setTheme(const Theme &thm)
{
    switch (thm) {
    case Theme::Panda_Light:
        scene_bgBrush = QColor("#ffffe6");
        scene_bgDots = QColor(Qt::darkGray);
        selectionBrush = QColor(175, 0, 0, 80);
        selectionPen = QColor(175, 0, 0, 255);

        graphicElement_labelColor = QColor(Qt::black);

        qneConnection_selected = selectionPen;

        qneConnection_true = QColor(Qt::green);
        qneConnection_false = QColor(Qt::darkGreen);
        qneConnection_invalid = QColor(Qt::red);

        break;
    case Theme::Panda_Dark:
        scene_bgBrush = QColor("#404552");
        scene_bgDots = QColor(Qt::black);
        selectionBrush = QColor(230, 255, 85, 150);
        selectionPen = QColor(230, 255, 85, 255);

        graphicElement_labelColor = QColor(Qt::white);

        qneConnection_selected = selectionPen;

        qneConnection_true = QColor(115, 255, 220, 255);
        qneConnection_false = QColor(65, 150, 130, 255);

        qneConnection_invalid = QColor(Qt::red);

        break;
    }
    qnePort_true_pen = QColor(Qt::black);
    qnePort_false_pen = QColor(Qt::black);
    qnePort_invalid_pen = QColor(Qt::red);

    qnePort_true_brush = qneConnection_true;
    qnePort_false_brush = qneConnection_false;
    qnePort_invalid_brush = qneConnection_invalid;

    qnePort_hoverPort = QColor(Qt::yellow);

    qnePort_output_pen = QColor(Qt::darkRed);
    qnePort_output_brush = QColor("#f35369");
}
