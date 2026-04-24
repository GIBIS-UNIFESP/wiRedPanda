// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Settings.h"

#include "App/Core/ThemeManager.h"

using namespace Qt::StringLiterals;

// ─── Backend helpers ────────────────────────────────────────────────────────

#ifdef USE_KDE_FRAMEWORKS

KConfigGroup Settings::groupFor(const QString &groupPath)
{
    const QStringList parts = groupPath.split(u'/');
    KConfigGroup g = KSharedConfig::openConfig()->group(parts.constFirst());
    for (int i = 1; i < parts.size(); ++i) {
        g = g.group(parts.at(i));
    }
    return g;
}

static void syncGroup(KConfigGroup &g)
{
    g.sync();
}

#else

QSettings *Settings::settingsInstance()
{
    if (!Settings::settings) {
#ifdef Q_OS_WASM
        // On WebAssembly, NativeFormat maps directly to browser localStorage,
        // providing persistent storage across sessions without filesystem sync.
        Settings::settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "GIBIS-UNIFESP", "wiRedPanda");
#else
        // IniFormat produces a human-readable text file rather than the platform registry
        // (Windows) or plist (macOS), making it easy to inspect and version-control settings.
        // UserScope stores the file in the user's home/config directory so that multiple
        // system users don't share the same preference file.
        Settings::settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GIBIS-UNIFESP", "wiRedPanda");
#endif
    }
    return Settings::settings;
}

QVariant Settings::value(const QString &key)
{
    return settingsInstance()->value(key);
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    settingsInstance()->setValue(key, value);
}

#endif // USE_KDE_FRAMEWORKS

// ─── fileName ───────────────────────────────────────────────────────────────

QString Settings::fileName()
{
#ifdef USE_KDE_FRAMEWORKS
    return KSharedConfig::openConfig()->name();
#else
    return settingsInstance()->fileName();
#endif
}

// ─── Window state ────────────────────────────────────────────────────────────

QByteArray Settings::mainWindowGeometry()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"MainWindow"_s).readEntry(u"geometry"_s, QByteArray{});
#else
    return value("MainWindow/geometry").toByteArray();
#endif
}

void Settings::setMainWindowGeometry(const QByteArray &geometry)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"MainWindow"_s);
    g.writeEntry(u"geometry"_s, geometry);
    syncGroup(g);
#else
    setValue("MainWindow/geometry", geometry);
#endif
}

QByteArray Settings::mainWindowState()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"MainWindow"_s).readEntry(u"windowState"_s, QByteArray{});
#else
    return value("MainWindow/windowState").toByteArray();
#endif
}

void Settings::setMainWindowState(const QByteArray &state)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"MainWindow"_s);
    g.writeEntry(u"windowState"_s, state);
    syncGroup(g);
#else
    setValue("MainWindow/windowState", state);
#endif
}

QByteArray Settings::splitterGeometry()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"MainWindow/splitter"_s).readEntry(u"geometry"_s, QByteArray{});
#else
    return value("MainWindow/splitter/geometry").toByteArray();
#endif
}

void Settings::setSplitterGeometry(const QByteArray &geometry)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"MainWindow/splitter"_s);
    g.writeEntry(u"geometry"_s, geometry);
    syncGroup(g);
#else
    setValue("MainWindow/splitter/geometry", geometry);
#endif
}

QByteArray Settings::splitterState()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"MainWindow/splitter"_s).readEntry(u"state"_s, QByteArray{});
#else
    return value("MainWindow/splitter/state").toByteArray();
#endif
}

void Settings::setSplitterState(const QByteArray &state)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"MainWindow/splitter"_s);
    g.writeEntry(u"state"_s, state);
    syncGroup(g);
#else
    setValue("MainWindow/splitter/state", state);
#endif
}

QByteArray Settings::dolphinGeometry()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"beWavedDolphin"_s).readEntry(u"geometry"_s, QByteArray{});
#else
    return value("beWavedDolphin/geometry").toByteArray();
#endif
}

void Settings::setDolphinGeometry(const QByteArray &geometry)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"beWavedDolphin"_s);
    g.writeEntry(u"geometry"_s, geometry);
    syncGroup(g);
#else
    setValue("beWavedDolphin/geometry", geometry);
#endif
}

// ─── UI preferences ──────────────────────────────────────────────────────────

bool Settings::fastMode()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"General"_s).readEntry(u"fastMode"_s, false);
#else
    return value("fastMode").toBool();
#endif
}

void Settings::setFastMode(bool enabled)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"General"_s);
    g.writeEntry(u"fastMode"_s, enabled);
    syncGroup(g);
#else
    setValue("fastMode", enabled);
#endif
}

bool Settings::labelsUnderIcons()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"General"_s).readEntry(u"labelsUnderIcons"_s, false);
#else
    return value("labelsUnderIcons").toBool();
#endif
}

void Settings::setLabelsUnderIcons(bool enabled)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"General"_s);
    g.writeEntry(u"labelsUnderIcons"_s, enabled);
    syncGroup(g);
#else
    setValue("labelsUnderIcons", enabled);
#endif
}

// ─── Language ────────────────────────────────────────────────────────────────

QString Settings::language()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"General"_s).readEntry(u"language"_s, QString{});
#else
    return value("language").toString();
#endif
}

void Settings::setLanguage(const QString &lang)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"General"_s);
    g.writeEntry(u"language"_s, lang);
    syncGroup(g);
#else
    setValue("language", lang);
#endif
}

// ─── Theme ───────────────────────────────────────────────────────────────────

Theme Settings::theme()
{
#ifdef USE_KDE_FRAMEWORKS
    const int saved = groupFor(u"General"_s).readEntry(u"theme"_s, static_cast<int>(Theme::System));
#else
    const QVariant v = value("theme");
    if (!v.isValid()) {
        return Theme::System;
    }
    const int saved = v.toInt();
#endif
    if (saved >= 0 && saved <= static_cast<int>(Theme::System)) {
        return static_cast<Theme>(saved);
    }
    return Theme::System;
}

void Settings::setTheme(Theme theme)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"General"_s);
    g.writeEntry(u"theme"_s, static_cast<int>(theme));
    syncGroup(g);
#else
    setValue("theme", static_cast<int>(theme));
#endif
}

// ─── Recent files ────────────────────────────────────────────────────────────

QStringList Settings::recentFiles()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"General"_s).readEntry(u"recentFileList"_s, QStringList{});
#else
    return value("recentFileList").toStringList();
#endif
}

void Settings::setRecentFiles(const QStringList &files)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"General"_s);
    g.writeEntry(u"recentFileList"_s, files);
    syncGroup(g);
#else
    setValue("recentFileList", files);
#endif
}

// ─── Autosave ────────────────────────────────────────────────────────────────

QStringList Settings::autosaveFiles()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"General"_s).readEntry(u"autosaveFile"_s, QStringList{});
#else
    return value("autosaveFile").toStringList();
#endif
}

void Settings::setAutosaveFiles(const QStringList &files)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"General"_s);
    g.writeEntry(u"autosaveFile"_s, files);
    syncGroup(g);
#else
    setValue("autosaveFile", files);
#endif
}

// ─── V4 format warning ───────────────────────────────────────────────────────

bool Settings::hideV4Warning()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"General"_s).readEntry(u"hideV4Warning"_s, false);
#else
    return value("hideV4Warning").toBool();
#endif
}

void Settings::setHideV4Warning(bool hide)
{
    if (hide) {
#ifdef USE_KDE_FRAMEWORKS
        KConfigGroup g = groupFor(u"General"_s);
        g.writeEntry(u"hideV4Warning"_s, true);
        syncGroup(g);
#else
        setValue("hideV4Warning", "true");
#endif
    } else {
#ifdef USE_KDE_FRAMEWORKS
        KConfigGroup g = groupFor(u"General"_s);
        g.deleteEntry(u"hideV4Warning"_s);
        syncGroup(g);
#else
        settingsInstance()->remove("hideV4Warning");
#endif
    }
}

// ─── Update checker ──────────────────────────────────────────────────────────

QString Settings::updateCheckLastDate()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"updateCheck"_s).readEntry(u"lastCheckDate"_s, QString{});
#else
    return value("updateCheck/lastCheckDate").toString();
#endif
}

void Settings::setUpdateCheckLastDate(const QString &date)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"updateCheck"_s);
    g.writeEntry(u"lastCheckDate"_s, date);
    syncGroup(g);
#else
    setValue("updateCheck/lastCheckDate", date);
#endif
}

QString Settings::updateCheckSkippedVersion()
{
#ifdef USE_KDE_FRAMEWORKS
    return groupFor(u"updateCheck"_s).readEntry(u"skippedVersion"_s, QString{});
#else
    return value("updateCheck/skippedVersion").toString();
#endif
}

void Settings::setUpdateCheckSkippedVersion(const QString &version)
{
#ifdef USE_KDE_FRAMEWORKS
    KConfigGroup g = groupFor(u"updateCheck"_s);
    g.writeEntry(u"skippedVersion"_s, version);
    syncGroup(g);
#else
    setValue("updateCheck/skippedVersion", version);
#endif
}
