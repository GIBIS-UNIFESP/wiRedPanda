// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thememanager.h"

#include "settings.h"

#include <QDebug>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    if (Settings::contains("theme")) {
        m_theme = static_cast<Theme>(Settings::value("theme").toInt());
    }

    m_attributes.setTheme(m_theme);
}

Theme ThemeManager::theme()
{
    return instance().m_theme;
}

void ThemeManager::setTheme(const Theme theme)
{
    instance().m_attributes.setTheme(theme);

    if (instance().m_theme == theme) {
        return;
    }

    instance().m_theme = theme;
    Settings::setValue("theme", static_cast<int>(theme));
    emit instance().themeChanged();
}

ThemeAttributes ThemeManager::attributes()
{
    return instance().m_attributes;
}

void ThemeAttributes::setTheme(const Theme theme)
{
    switch (theme) {
    case Theme::Light: {
        m_sceneBgBrush = QColor(255, 255, 230);
        m_sceneBgDots = QColor(Qt::darkGray);

        m_selectionBrush = QColor(175, 0, 0, 80);
        m_selectionPen = QColor(175, 0, 0, 255);

        m_graphicElementLabelColor = QColor(Qt::black);

        m_qneConnectionFalse = QColor(Qt::darkGreen);
        m_qneConnectionInvalid = QColor(Qt::red);
        m_qneConnectionSelected = m_selectionPen;
        m_qneConnectionTrue = QColor(Qt::green);

#ifndef Q_OS_MAC
        QPalette lightPalette = m_defaultPalette;
        lightPalette.setColor(QPalette::AlternateBase, QColor("#e9e7e3"));

        qApp->setPalette(lightPalette);
#endif

        break;
    }

    case Theme::Dark: {
        m_sceneBgBrush = QColor(64, 69, 82);
        m_sceneBgDots = QColor(Qt::lightGray);

        m_selectionBrush = QColor(230, 255, 85, 150);
        m_selectionPen = QColor(230, 255, 85, 255);

        m_graphicElementLabelColor = QColor(Qt::white);

        m_qneConnectionFalse = QColor(65, 150, 130, 255);
        m_qneConnectionInvalid = QColor(Qt::red);
        m_qneConnectionSelected = m_selectionPen;
        m_qneConnectionTrue = QColor(115, 255, 220, 255);

#ifndef Q_OS_MAC
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);

        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
        darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(120, 120, 120));
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 120, 120));

        qApp->setPalette(darkPalette);
#endif

        break;
    }
    }

#ifndef Q_OS_MAC
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
#endif

    m_qnePortFalseBrush = m_qneConnectionFalse;
    m_qnePortInvalidBrush = m_qneConnectionInvalid;
    m_qnePortOutputBrush = QColor(243, 83, 105);
    m_qnePortTrueBrush = m_qneConnectionTrue;

    m_qnePortFalsePen = QColor(Qt::black);
    m_qnePortInvalidPen = QColor(Qt::red);
    m_qnePortOutputPen = QColor(Qt::darkRed);
    m_qnePortTruePen = QColor(Qt::black);

    m_qnePortHoverPort = QColor(Qt::yellow);
}
