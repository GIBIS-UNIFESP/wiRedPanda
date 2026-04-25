// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ThemeManager.h"

#include <QCoreApplication>
#include <QStyleHints>

#include "App/Core/Settings.h"

#ifdef USE_KDE_FRAMEWORKS
#include <KColorScheme>
#endif

using namespace Qt::StringLiterals;

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    // Load the persisted theme preference; if not set, m_theme keeps its
    // default-initialised value (Theme::Light, as defined in the header)
    m_theme = Settings::theme();

    // Apply the theme immediately so colours are correct before any widgets are shown.
    // Resolve System here directly — effectiveTheme() would re-enter instance() and deadlock
    // because the static local is still being constructed.
    // KDE: pass Theme::System through unresolved so ThemeAttributes can query KColorScheme.
#ifdef USE_KDE_FRAMEWORKS
    const Theme effective = m_theme;
#else
    const Theme effective = (m_theme == Theme::System) ? resolveSystemTheme() : m_theme;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // Set color scheme BEFORE applying the palette: setColorScheme() controls what
    // QApplication::style()->standardPalette() returns on Windows/Qt 6.8+.
    // Calling it after would cause the Light theme to read a dark standardPalette().
    // System theme: leave m_requestedColorScheme as Unknown so the platform reads the
    // OS color scheme naturally. Setting it to Dark/Light explicitly would block runtime
    // OS theme-change events from propagating through QGtk3Theme::colorScheme().
    if (auto *app = qApp) {
        switch (m_theme) {
        case Theme::Light:  app->styleHints()->setColorScheme(Qt::ColorScheme::Light);  break;
        case Theme::Dark:   app->styleHints()->setColorScheme(Qt::ColorScheme::Dark);   break;
        case Theme::System: break; // leave Unknown — platform reads OS setting directly
        }
    }
#endif

    m_attributes.setTheme(effective);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // Qt 6.5+ provides a built-in color-scheme change signal; use it so System
    // theme reacts at runtime when the user toggles OS dark/light mode.
    if (auto *app = qApp) {
        connect(app->styleHints(), &QStyleHints::colorSchemeChanged,
                this, [this](Qt::ColorScheme) { onSystemColorSchemeChanged(); });
    }
#endif
}

QString ThemeManager::themePath()
{
#ifdef USE_KDE_FRAMEWORKS
    if (instance().m_theme == Theme::System) {
        const KColorScheme view(QPalette::Active, KColorScheme::View);
        return (view.background().color().lightness() < 128) ? u"Dark"_s : u"Light"_s;
    }
#endif
    return (effectiveTheme() == Theme::Light) ? u"Light"_s : u"Dark"_s;
}

Theme ThemeManager::theme()
{
    return instance().m_theme;
}

Theme ThemeManager::effectiveTheme()
{
    const Theme t = instance().m_theme;
    if (t != Theme::System) {
        return t;
    }
#ifdef USE_KDE_FRAMEWORKS
    const KColorScheme view(QPalette::Active, KColorScheme::View);
    return (view.background().color().lightness() < 128) ? Theme::Dark : Theme::Light;
#else
    return resolveSystemTheme();
#endif
}

Theme ThemeManager::resolveSystemTheme()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (auto *app = qApp) {
        return (app->styleHints()->colorScheme() == Qt::ColorScheme::Dark) ? Theme::Dark : Theme::Light;
    }
    return Theme::Light;
#else
    if (auto *app = qApp) {
        return (app->palette().color(QPalette::Window).lightness() < 128)
                   ? Theme::Dark : Theme::Light;
    }
    return Theme::Light;
#endif
}

void ThemeManager::onSystemColorSchemeChanged()
{
    if (m_theme == Theme::System) {
#ifdef USE_KDE_FRAMEWORKS
        // Pass Theme::System through so KColorScheme re-queries the live palette.
        m_attributes.setTheme(Theme::System);
#else
        m_attributes.setTheme(resolveSystemTheme());
#endif
        emit themeChanged();
    }
}

void ThemeManager::setTheme(const Theme theme)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // For System theme, clear the explicit color scheme override first so the platform
    // re-reads the OS setting. Qt 6.9.3's requestColorScheme() posts a QWindowSystemInterface
    // event but doesn't flush it — QStyleHints::colorScheme() would return a stale cached
    // value until the event loop runs. Flushing here replicates the fix that Qt 6.10.0
    // added directly inside requestColorScheme().
    if (theme == Theme::System) {
        if (auto *app = Application::instance()) {
            app->styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
            // Flush the pending QWindowSystemInterface::handleThemeChange() event so
            // QStyleHintsPrivate::m_colorScheme is updated before we call resolveSystemTheme().
            // Qt 6.10.0 fixed this inside requestColorScheme() itself (commit 2fe9eed3fdd5,
            // "QGnomeTheme, QGtk3Theme: Refactor and Simplify DBus Interactions", 2025-05-30)
            // by adding QWindowSystemInterface::sendWindowSystemEvents(QEventLoop::AllEvents).
            // On Qt 6.9.x the flush is absent, leaving the cache stale until the event loop runs.
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }
#endif

    // KDE: pass Theme::System through unresolved so ThemeAttributes::setTheme can route
    // it to the KColorScheme path. On non-KDE builds, resolve to Light/Dark up-front.
#ifdef USE_KDE_FRAMEWORKS
    const Theme effective = theme;
#else
    const Theme effective = (theme == Theme::System) ? resolveSystemTheme() : theme;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // Set color scheme BEFORE applying the palette: on Windows/Qt 6.8+,
    // setColorScheme() controls what QApplication::style()->standardPalette() returns.
    // Calling it after would cause the Light theme to read a dark standardPalette().
    // System theme: m_requestedColorScheme is already Unknown from the block above —
    // leave it there. Re-setting it to Dark/Light would break runtime OS theme-change
    // detection (QGtk3Theme::colorScheme() would return the explicit value, ignoring
    // any subsequent OS toggle).
    if (auto *app = Application::instance()) {
        switch (theme) {
        case Theme::Light:  app->styleHints()->setColorScheme(Qt::ColorScheme::Light);  break;
        case Theme::Dark:   app->styleHints()->setColorScheme(Qt::ColorScheme::Dark);   break;
        case Theme::System: break; // already Unknown — platform reads OS setting directly
        }
    }
#endif

    // Always refresh ThemeAttributes so palette and color constants are current,
    // even if the theme value itself hasn't changed (e.g. initial load).
    instance().m_attributes.setTheme(effective);

    // Early-exit after refreshing attributes: don't re-emit themeChanged() or
    // re-write to settings when the theme is the same (avoids unnecessary repaints)
    if (instance().m_theme == theme) {
        return;
    }

    instance().m_theme = theme;
    // Persist so the selected theme is restored on next application launch
    Settings::setTheme(theme);
    // Notify all connected widgets (scene, view, etc.) to repaint with the new palette
    emit instance().themeChanged();
}

ThemeAttributes ThemeManager::attributes()
{
    return instance().m_attributes;
}

void ThemeAttributes::setTheme(const Theme theme)
{
#ifdef USE_KDE_FRAMEWORKS
    if (theme == Theme::System) {
        // Derive scene colors from KColorScheme so the canvas blends with the
        // user's KDE/Plasma color scheme. The Qt palette is left untouched —
        // Plasma owns it. Light/Dark explicit overrides keep the hardcoded values
        // below so users still get a deterministic look when they pick one.
        const KColorScheme view(QPalette::Active, KColorScheme::View);
        const KColorScheme selection(QPalette::Active, KColorScheme::Selection);

        m_sceneBgBrush = view.background().color();
        m_sceneBgDots = view.foreground(KColorScheme::InactiveText).color();

        const QColor selBg = selection.background().color();
        m_selectionBrush = QColor(selBg.red(), selBg.green(), selBg.blue(), 80);
        m_selectionPen = selBg;

        m_graphicElementLabelColor = view.foreground().color();

        m_connectionUnknown  = view.foreground(KColorScheme::InactiveText).color();
        m_connectionInactive = view.foreground(KColorScheme::NeutralText).color();
        m_connectionActive   = view.foreground(KColorScheme::PositiveText).color();
        m_connectionError    = view.foreground(KColorScheme::NegativeText).color();
        m_connectionSelected = m_selectionPen;

        m_portUnknownBrush  = m_connectionUnknown;
        m_portInactiveBrush = m_connectionInactive;
        m_portActiveBrush   = m_connectionActive;
        m_portErrorBrush    = m_connectionError;
        m_portOutputBrush   = view.foreground(KColorScheme::LinkText).color();

        m_portUnknownPen  = view.foreground(KColorScheme::InactiveText).color();
        m_portInactivePen = view.foreground().color();
        m_portActivePen   = view.foreground().color();
        m_portErrorPen    = view.foreground(KColorScheme::NegativeText).color();
        m_portOutputPen   = m_portOutputBrush.darker();

        m_portHoverPort = view.decoration(KColorScheme::HoverColor).color();
        return;
    }
#endif

    switch (theme) {
    case Theme::Light: {
        m_sceneBgBrush = QColor(255, 255, 230); // warm off-white (slight yellow tint); less eye-strain than pure white
        m_sceneBgDots = QColor(Qt::darkGray);

        m_selectionBrush = QColor(175, 0, 0, 80);  // semi-transparent red fill for rubber-band selection
        m_selectionPen = QColor(175, 0, 0, 255);   // fully-opaque red border for the selection rectangle

        m_graphicElementLabelColor = QColor(Qt::black);

        m_connectionUnknown = QColor(140, 140, 140);    // neutral gray — "nothing is driving this"
        m_connectionInactive = QColor(Qt::darkGreen);
        m_connectionActive = QColor(Qt::green);
        m_connectionSelected = m_selectionPen;

#ifndef Q_OS_MAC
        // Build light palette from scratch, independent of the current system
        // theme state. This ensures Light theme works correctly even when the
        // system is in dark mode.
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, Qt::white);
        lightPalette.setColor(QPalette::WindowText, Qt::black);
        lightPalette.setColor(QPalette::Base, Qt::white);
        lightPalette.setColor(QPalette::AlternateBase, QColor(233, 231, 227)); // light grey for alternating rows
        lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
        lightPalette.setColor(QPalette::ToolTipText, Qt::black);
        lightPalette.setColor(QPalette::Text, Qt::black);
        lightPalette.setColor(QPalette::Button, Qt::white);
        lightPalette.setColor(QPalette::ButtonText, Qt::black);
        lightPalette.setColor(QPalette::BrightText, Qt::red);
        lightPalette.setColor(QPalette::Link, QColor(0, 0, 255));
        lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215)); // light blue selection
        lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
        lightPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));

        if (Application::instance()) {
            Application::instance()->setPalette(lightPalette);
        }
#endif

        break;
    }

    case Theme::Dark: {
        m_sceneBgBrush = QColor(64, 69, 82); // dark blue-grey slate; chosen to provide sufficient luminance
                                             // contrast against the light-grey dot grid overlay
        m_sceneBgDots = QColor(Qt::lightGray);

        m_selectionBrush = QColor(230, 255, 85, 150); // semi-transparent yellow-green fill; contrasts with dark background
        m_selectionPen = QColor(230, 255, 85, 255);   // fully-opaque for a crisp selection border

        m_graphicElementLabelColor = QColor(Qt::white);

        m_connectionUnknown = QColor(160, 160, 160, 255);    // light gray — "nothing is driving this"
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

        if (Application::instance()) {
            Application::instance()->setPalette(darkPalette);
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
    if (Application::instance()) {
        Application::instance()->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
#endif

    // Port brushes mirror the wire colours for consistency so users can visually
    // correlate a wire's colour with the port it connects to.
    // Output ports use a distinct pink/red hue to visually distinguish them from input ports.
    m_portUnknownBrush = m_connectionUnknown;
    m_portInactiveBrush = m_connectionInactive;
    m_portActiveBrush = m_connectionActive;
    m_portErrorBrush = m_connectionError;
    m_portOutputBrush = QColor(243, 83, 105); // vivid rose-pink for output ports

    // Port pen colours are theme-invariant (always black/darkRed) because they are drawn
    // as outlines on top of the brush fill and need consistent contrast
    m_portUnknownPen = QColor(120, 120, 120);  // dark gray — distinct from error red
    m_portInactivePen = QColor(Qt::black);
    m_portActivePen = QColor(Qt::black);
    m_portErrorPen = QColor(Qt::red);
    m_portOutputPen = QColor(Qt::darkRed);

    m_portHoverPort = QColor(Qt::yellow); // bright yellow on hover for high visibility regardless of theme
}

