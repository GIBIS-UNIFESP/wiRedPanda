// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/SystemThemeDetector.h"

#include <QApplication>
#include <QEvent>
#include <QPalette>

#ifdef HAVE_QTDBUS
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusVariant>
#endif

#ifdef Q_OS_WIN
#include <QSettings>
#endif

SystemThemeDetector &SystemThemeDetector::instance()
{
    static SystemThemeDetector inst;
    return inst;
}

SystemThemeDetector::SystemThemeDetector(QObject *parent)
    : QObject(parent)
{
    m_cached = queryPlatform();
    setupListeners();
}

ColorScheme SystemThemeDetector::colorScheme()
{
    return instance().m_cached;
}

bool SystemThemeDetector::isDark()
{
    return colorScheme() == ColorScheme::Dark;
}

void SystemThemeDetector::handleChange(ColorScheme newScheme)
{
    if (newScheme != m_cached) {
        m_cached = newScheme;
        emit colorSchemeChanged(newScheme);
    }
}

// ---------------------------------------------------------------------------
// Platform-specific: Linux with D-Bus (freedesktop portal)
// ---------------------------------------------------------------------------

#ifdef HAVE_QTDBUS

/// Freedesktop portal color-scheme values: 0 = no preference, 1 = dark, 2 = light.
static ColorScheme fromPortalValue(uint value)
{
    switch (value) {
    case 1:  return ColorScheme::Dark;
    case 2:  return ColorScheme::Light;
    default: return ColorScheme::Light; // 0 ("no preference") defaults to Light
    }
}

static ColorScheme queryDbus()
{
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        return ColorScheme::Unknown;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        "Read");
    msg << "org.freedesktop.appearance" << "color-scheme";

    QDBusReply<QDBusVariant> reply = bus.call(msg, QDBus::Block, 250);
    if (!reply.isValid()) {
        return ColorScheme::Unknown;
    }

    // The portal returns variant<variant<uint>>; unwrap both layers.
    const QVariant outer = reply.value().variant();
    const uint value = outer.canConvert<QDBusVariant>()
        ? outer.value<QDBusVariant>().variant().toUInt()
        : outer.toUInt();

    return fromPortalValue(value);
}

void SystemThemeDetector::onDbusSettingChanged(
    const QString &ns, const QString &key, const QDBusVariant &value)
{
    if (ns == "org.freedesktop.appearance" && key == "color-scheme") {
        handleChange(fromPortalValue(value.variant().toUInt()));
    }
}

#endif // HAVE_QTDBUS

// ---------------------------------------------------------------------------
// queryPlatform() — per-platform one-shot query
// ---------------------------------------------------------------------------

#ifdef Q_OS_WIN

ColorScheme SystemThemeDetector::queryPlatform()
{
    const QSettings reg(
        R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",
        QSettings::NativeFormat);

    const QVariant val = reg.value("AppsUseLightTheme");
    if (!val.isValid()) {
        return ColorScheme::Unknown;
    }
    // 0 = dark, 1 = light
    return val.toUInt() == 0 ? ColorScheme::Dark : ColorScheme::Light;
}

#elif defined(HAVE_QTDBUS)

ColorScheme SystemThemeDetector::queryPlatform()
{
    const auto result = queryDbus();
    if (result != ColorScheme::Unknown) {
        return result;
    }
    // D-Bus portal unavailable at runtime — fall back to palette heuristic.
    if (auto *app = qApp) {
        const int lightness = app->palette().color(QPalette::Window).lightness();
        return (lightness < 128) ? ColorScheme::Dark : ColorScheme::Light;
    }
    return ColorScheme::Unknown;
}

#elif defined(Q_OS_WASM)

#include <emscripten.h>

ColorScheme SystemThemeDetector::queryPlatform()
{
    // Query the browser's prefers-color-scheme media query.
    const bool dark = EM_ASM_INT({
        return window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches ? 1 : 0;
    });
    return dark ? ColorScheme::Dark : ColorScheme::Light;
}

#else

ColorScheme SystemThemeDetector::queryPlatform()
{
    // macOS, Linux without D-Bus, and other platforms: palette heuristic.
    // Qt on macOS applies NSApp.effectiveAppearance to the default palette,
    // so the lightness check is reliable there.
    if (auto *app = qApp) {
        const int lightness = app->palette().color(QPalette::Window).lightness();
        return (lightness < 128) ? ColorScheme::Dark : ColorScheme::Light;
    }
    return ColorScheme::Unknown;
}

#endif

// ---------------------------------------------------------------------------
// setupListeners() + eventFilter — live-change monitoring
// ---------------------------------------------------------------------------

#ifdef Q_OS_WASM
#include <emscripten/bind.h>

static void wasmColorSchemeChanged(int isDark)
{
    SystemThemeDetector::instance().handleChange(isDark ? ColorScheme::Dark : ColorScheme::Light);
}

EMSCRIPTEN_BINDINGS(theme_detector) {
    emscripten::function("_wpanda_onColorSchemeChanged", &wasmColorSchemeChanged);
}
#endif

void SystemThemeDetector::setupListeners()
{
#ifdef HAVE_QTDBUS
    auto bus = QDBusConnection::sessionBus();
    if (bus.isConnected()) {
        // Subscribe to org.freedesktop.portal.Settings.SettingChanged
        bus.connect(
            "org.freedesktop.portal.Desktop",
            "/org/freedesktop/portal/desktop",
            "org.freedesktop.portal.Settings",
            "SettingChanged",
            this,
            SLOT(onDbusSettingChanged(QString,QString,QDBusVariant)));
    }
#endif

#ifdef Q_OS_WASM
    // Listen for browser theme changes via matchMedia.
    EM_ASM({
        var mq = window.matchMedia('(prefers-color-scheme: dark)');
        mq.addEventListener('change', function(e) {
            Module._wpanda_onColorSchemeChanged(e.matches ? 1 : 0);
        });
    });
#endif

    // Install an event filter on QApplication to catch palette changes.
    // This works on all Qt versions (unlike the deprecated paletteChanged signal)
    // and covers Windows (WM_SETTINGCHANGE), macOS (NSAppearance), and fallback.
    if (auto *app = qApp) {
        app->installEventFilter(this);
    }
}

bool SystemThemeDetector::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        handleChange(queryPlatform());
    }
    return QObject::eventFilter(obj, event);
}

