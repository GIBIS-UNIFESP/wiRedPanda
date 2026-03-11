// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ThemeManager.h"

#include <QDebug>

#include "App/Core/Settings.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QStyleHints>
#endif

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    // Load the persisted theme preference; if not set, m_theme keeps its
    // default-initialised value (Theme::Light, as defined in the header)
    if (Settings::contains("theme")) {
        m_theme = static_cast<Theme>(Settings::value("theme").toInt());
    }

    // Apply the theme immediately so colours are correct before any widgets are shown
    m_attributes.setTheme(m_theme);
}

QString ThemeManager::themePath()
{
    return (ThemeManager::theme() == Theme::Light) ? "Light" : "Dark";
}

Theme ThemeManager::theme()
{
    return instance().m_theme;
}

void ThemeManager::setTheme(const Theme theme)
{
    // Always refresh ThemeAttributes so palette and color constants are current,
    // even if the theme value itself hasn't changed (e.g. initial load).
    instance().m_attributes.setTheme(theme);

    // Early-exit after refreshing attributes: don't re-emit themeChanged() or
    // re-write to settings when the theme is the same (avoids unnecessary repaints)
    if (instance().m_theme == theme) {
        return;
    }

    instance().m_theme = theme;
    // Persist so the selected theme is restored on next application launch
    Settings::setValue("theme", static_cast<int>(theme));
    // Notify all connected widgets (scene, view, etc.) to repaint with the new palette
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
        // Qt 6.8+ honours the color scheme hint at the platform level,
        // enabling OS-native light/dark integration for system controls.
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif

        m_sceneBgBrush = QColor(255, 255, 230); // warm off-white (slight yellow tint); less eye-strain than pure white
        m_sceneBgDots = QColor(Qt::darkGray);

        m_selectionBrush = QColor(175, 0, 0, 80);  // semi-transparent red fill for rubber-band selection
        m_selectionPen = QColor(175, 0, 0, 255);   // fully-opaque red border for the selection rectangle

        m_graphicElementLabelColor = QColor(Qt::black);

        m_connectionInactive = QColor(Qt::darkGreen);
        m_connectionActive = QColor(Qt::green);
        m_connectionSelected = m_selectionPen;

#ifndef Q_OS_MAC
        // Start from the platform default palette so that unset roles (e.g. scroll bars,
        // dialog backgrounds) keep their native appearance; only override AlternateBase
        // to give table/list rows a subtle banding colour
        QPalette lightPalette = m_defaultPalette;
        lightPalette.setColor(QPalette::AlternateBase, QColor(233, 231, 227)); // light grey for alternating rows

        qApp->setPalette(lightPalette);
#endif

        break;
    }

    case Theme::Dark: {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
#endif

        m_sceneBgBrush = QColor(64, 69, 82); // dark blue-grey slate; chosen to provide sufficient luminance
                                             // contrast against the light-grey dot grid overlay
        m_sceneBgDots = QColor(Qt::lightGray);

        m_selectionBrush = QColor(230, 255, 85, 150); // semi-transparent yellow-green fill; contrasts with dark background
        m_selectionPen = QColor(230, 255, 85, 255);   // fully-opaque for a crisp selection border

        m_graphicElementLabelColor = QColor(Qt::white);

        m_connectionInactive = QColor(65, 150, 130, 255);  // muted teal; visible on dark background without competing with active wires
        m_connectionActive = QColor(115, 255, 220, 255);    // bright cyan-green; clearly distinguishes an asserted wire
        m_connectionSelected = m_selectionPen;

#ifndef Q_OS_MAC
        // macOS handles dark mode at the OS level; manually overriding the
        // palette there causes visual glitches, so the block is skipped.
        QPalette darkPalette;
        // A standard Qt dark palette recipe: 53/53/53 for surfaces, 25/25/25 for
        // input fields (darker so they stand out from the surrounding surface).
        // 120/120/120 is used for disabled roles — roughly mid-grey, readable but
        // clearly de-emphasised compared to the full-white active text.
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));       // input field / list background
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);             // used by some styles for error text
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));      // blue link colour matching Highlight

        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218)); // selection highlight (mid-blue)

        // Disabled widgets use a uniform mid-grey to signal unavailability without
        // disappearing entirely (white on dark would be too low-contrast)
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
        darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(120, 120, 120));
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 120, 120));

        if (qApp) {
            qApp->setPalette(darkPalette);
        }
#endif

        break;
    }

    default:
        // Handle unexpected theme values gracefully - fallback to Light theme
        setTheme(Theme::Light);
        break;
    }

#ifndef Q_OS_MAC
    // Force a consistent tooltip style on both themes; without this, Windows/Linux
    // style sheets would show platform-default tooltips that are illegible on dark
    // backgrounds.  The #2a82da background matches QPalette::Highlight above.
    if (qApp) {
        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
#endif

    // Port brushes mirror the wire colours for consistency so users can visually
    // correlate a wire's colour with the port it connects to.
    // Output ports use a distinct pink/red hue to visually distinguish them from input ports.
    m_portInvalidBrush = m_connectionInvalid;
    m_portInactiveBrush = m_connectionInactive;
    m_portActiveBrush = m_connectionActive;
    m_portOutputBrush = QColor(243, 83, 105); // vivid rose-pink for output ports

    // Port pen colours are theme-invariant (always black/darkRed) because they are drawn
    // as outlines on top of the brush fill and need consistent contrast
    m_portInvalidPen = QColor(Qt::red);
    m_portInactivePen = QColor(Qt::black);
    m_portActivePen = QColor(Qt::black);
    m_portOutputPen = QColor(Qt::darkRed);

    m_portHoverPort = QColor(Qt::yellow); // bright yellow on hover for high visibility regardless of theme
}
