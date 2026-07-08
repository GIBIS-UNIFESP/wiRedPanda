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

QString InstallRelativePaths::resolve(const QString &category)
{
    for (const QString &candidate : candidates(category)) {
        if (!candidate.isEmpty() && QDir(candidate).exists()) {
            return candidate;
        }
    }
    return {};
}
