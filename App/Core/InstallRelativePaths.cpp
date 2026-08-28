// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/InstallRelativePaths.h"

#include <QCoreApplication>
#include <QDir>

QStringList InstallRelativePaths::candidates(const QString &category)
{
    const QString appDir = QCoreApplication::applicationDirPath();

    QStringList result = {
        appDir + QLatin1Char('/') + category,                       // Windows / dev builds
    };
#ifdef Q_OS_MACOS
    result << appDir + QStringLiteral("/../Resources/") + category; // macOS app bundle
#endif
#ifdef Q_OS_LINUX
    result << qEnvironmentVariable("APPDIR") + QStringLiteral("/usr/share/wiredpanda/") + category; // AppImage
    result << appDir + QStringLiteral("/../share/wiredpanda/") + category; // native FHS install (bin/ -> share/wiredpanda/<category>)
#endif
#ifdef Q_OS_WASM
    result << QStringLiteral("/") + category;                       // WASM virtual filesystem
#endif
    result << category;                                              // CWD fallback (development)

    return result;
}

bool InstallRelativePaths::isCandidate(const QString &category, const QString &directory)
{
    // canonicalPath() resolves "..", symlinks and CWD-relative entries to one comparable
    // form, and returns "" for a path that doesn't exist -- which is also the right answer
    // here, since a directory that isn't there can't be the bundled content directory.
    const QString target = QDir(directory).canonicalPath();
    if (target.isEmpty()) {
        return false;
    }

    const auto candidateDirs = candidates(category);
    for (const QString &candidate : candidateDirs) {
        if (candidate.isEmpty()) {
            continue;
        }
        if (QDir(candidate).canonicalPath() == target) {
            return true;
        }
    }
    return false;
}

QString InstallRelativePaths::resolve(const QString &category)
{
    for (const QString &candidate : candidates(category)) {
        if (!candidate.isEmpty() && QDir(candidate).exists()) {
            return candidate;
        }
    }
    return {};
}
