// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ThemeManager.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QStyleHints>
#include <QThread>

#include "App/Core/Settings.h"

ThemeManager::WidgetsStylingApplier ThemeManager::s_widgetsStylingApplier;

void ThemeManager::setWidgetsStylingApplier(WidgetsStylingApplier applier)
{
    s_widgetsStylingApplier = std::move(applier);
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());

    // Load the persisted theme preference; if not set, m_theme keeps its
    // default-initialised value (Theme::Light, as defined in the header)
    m_theme = Settings::theme();

    // Apply the theme immediately so colours are correct before any widgets are shown.
    // Resolve System here directly — effectiveTheme() would re-enter instance() and deadlock
    // because the static local is still being constructed.
    const Theme effective = (m_theme == Theme::System) ? resolveSystemTheme() : m_theme;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // Set color scheme BEFORE applying the palette: setColorScheme() controls what
    // QApplication::style()->standardPalette() returns on Windows/Qt 6.8+.
    // Calling it after would cause the Light theme to read a dark standardPalette().
    // System theme: leave m_requestedColorScheme as Unknown so the platform reads the
    // OS color scheme naturally. Setting it to Dark/Light explicitly would block runtime
    // OS theme-change events from propagating through QGtk3Theme::colorScheme().
    if (auto *app = qApp) {
        // Unreachable in the test suite: ThemeManager::instance() is a Meyer's
        // singleton constructed exactly once, the first time anything in the
        // whole test binary touches it — at that moment the (redirected) test
        // settings store has never had a theme persisted yet, so m_theme is
        // always Theme::System here. Reaching Light/Dark would need a
        // persisted preference to already exist before the singleton's very
        // first construction, which no test can arrange after the fact.
        switch (m_theme) {
        case Theme::Light:  app->styleHints()->setColorScheme(Qt::ColorScheme::Light);  break; // LCOV_EXCL_LINE
        case Theme::Dark:   app->styleHints()->setColorScheme(Qt::ColorScheme::Dark);   break; // LCOV_EXCL_LINE
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
    return (effectiveTheme() == Theme::Light) ? "Light" : "Dark";
}

Theme ThemeManager::theme()
{
    return instance().m_theme;
}

Theme ThemeManager::effectiveTheme()
{
    const Theme t = instance().m_theme;
    return (t == Theme::System) ? resolveSystemTheme() : t;
}

Theme ThemeManager::resolveSystemTheme()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (auto *app = qApp) {
        return (app->styleHints()->colorScheme() == Qt::ColorScheme::Dark) ? Theme::Dark : Theme::Light;
    }
    return Theme::Light; // LCOV_EXCL_LINE -- qApp is always alive while any test runs
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
        m_attributes.setTheme(resolveSystemTheme());
        emit themeChanged();
    }
}

void ThemeManager::setTheme(const Theme theme)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // For System theme, clear the explicit color scheme override first so the platform
    // re-reads the OS setting. Qt 6.9.3's requestColorScheme() posts a QWindowSystemInterface
    // event but doesn't flush it — QStyleHints::colorScheme() would return a stale cached
    // value until the event loop runs. Flushing here replicates the fix that Qt 6.10.0
    // added directly inside requestColorScheme().
    if (theme == Theme::System) {
        if (auto *app = qGuiApp) {
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

    const Theme effective = (theme == Theme::System) ? resolveSystemTheme() : theme;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // Set color scheme BEFORE applying the palette: on Windows/Qt 6.8+,
    // setColorScheme() controls what QApplication::style()->standardPalette() returns.
    // Calling it after would cause the Light theme to read a dark standardPalette().
    // System theme: m_requestedColorScheme is already Unknown from the block above —
    // leave it there. Re-setting it to Dark/Light would break runtime OS theme-change
    // detection (QGtk3Theme::colorScheme() would return the explicit value, ignoring
    // any subsequent OS toggle).
    if (auto *app = qGuiApp) {
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

    if (s_widgetsStylingApplier) {
        s_widgetsStylingApplier(effective);
    }

    // Notify all connected widgets (scene, view, etc.) to repaint with the new palette
    emit instance().themeChanged();
}

const ThemeAttributes &ThemeManager::attributes()
{
    return instance().m_attributes;
}

void ThemeAttributes::setTheme(const Theme theme)
{
    switch (theme) {
    case Theme::Light: {
        m_sceneBgBrush = QColor(255, 255, 230); // warm off-white (slight yellow tint); less eye-strain than pure white
        m_sceneBgDots = QColor(Qt::darkGray);

        m_selectionBrush = QColor(175, 0, 0, 80);  // semi-transparent red fill for rubber-band selection
        m_selectionPen = QColor(175, 0, 0, 255);   // fully-opaque red border for the selection rectangle

        m_graphicElementLabelColor = QColor(Qt::black);

        m_minimapElementBrush = QColor(120, 120, 120); // mid-gray; contrasts with the warm off-white background

        m_connectionUnknown = QColor(140, 140, 140);    // neutral gray — "nothing is driving this"
        m_connectionInactive = QColor(Qt::darkGreen);
        m_connectionActive = QColor(Qt::green);
        m_connectionSelected = m_selectionPen;

        break;
    }

    case Theme::Dark: {
        m_sceneBgBrush = QColor(64, 69, 82); // dark blue-grey slate; chosen to provide sufficient luminance
                                             // contrast against the light-grey dot grid overlay
        m_sceneBgDots = QColor(Qt::lightGray);

        m_selectionBrush = QColor(230, 255, 85, 150); // semi-transparent yellow-green fill; contrasts with dark background
        m_selectionPen = QColor(230, 255, 85, 255);   // fully-opaque for a crisp selection border

        m_graphicElementLabelColor = QColor(Qt::white);

        m_minimapElementBrush = QColor(150, 155, 165); // light-blue-grey; contrasts with the dark blue-grey background

        m_connectionUnknown = QColor(160, 160, 160, 255);    // light gray — "nothing is driving this"
        m_connectionInactive = QColor(65, 150, 130, 255);  // muted teal; visible on dark background without competing with active wires
        m_connectionActive = QColor(115, 255, 220, 255);    // bright cyan-green; clearly distinguishes an asserted wire
        m_connectionSelected = m_selectionPen;

        break;
    }

    default:
        // Handle unexpected theme values gracefully - fallback to Light theme
        setTheme(Theme::Light);
        break;
    }

    // Port brushes mirror the wire colours for consistency so users can visually
    // correlate a wire's colour with the port it connects to.
    m_portUnknownBrush = m_connectionUnknown;
    m_portInactiveBrush = m_connectionInactive;
    m_portActiveBrush = m_connectionActive;
    m_portErrorBrush = m_connectionError;

    // Port pen colours are theme-invariant because they are drawn as outlines
    // on top of the brush fill and need consistent contrast
    m_portUnknownPen = QColor(120, 120, 120);  // dark gray — distinct from error red
    m_portInactivePen = QColor(Qt::black);
    m_portActivePen = QColor(Qt::black);
    m_portErrorPen = QColor(Qt::red);

    m_portHoverPort = QColor(Qt::yellow); // bright yellow on hover for high visibility regardless of theme

    // Hover-label chips reuse the forced tooltip palette (see the QToolTip stylesheet above):
    // #2a82da background with white text, theme-invariant so the labels read clearly on both themes.
    m_portHoverLabelBg = QColor(0x2a, 0x82, 0xda);
    m_portHoverLabelText = QColor(Qt::white);
}
