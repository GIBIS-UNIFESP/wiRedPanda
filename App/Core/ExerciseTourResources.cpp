// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ExerciseTourResources.h"

#include <algorithm>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>

#include "App/Core/Settings.h"

QVector<ExerciseTourResourceEntry> ExerciseTourResources::scan(const QString &directory)
{
    QVector<ExerciseTourResourceEntry> entries;

    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "ExerciseTourResources::scan: directory not found:" << directory;
        return entries;
    }

    const QStringList jsonFiles = dir.entryList({QStringLiteral("*.json")}, QDir::Files);
    for (const QString &fileName : jsonFiles) {
        const QString path = directory + QLatin1Char('/') + fileName;

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "ExerciseTourResources::scan: failed to open" << path;
            continue;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull() || !doc.isObject()) {
            qWarning() << "ExerciseTourResources::scan: malformed JSON in" << path;
            continue;
        }

        const QJsonObject root = doc.object();
        const QString id = root.value(QStringLiteral("id")).toString();
        const QString title = root.value(QStringLiteral("title")).toString();
        if (id.isEmpty() || title.isEmpty()) {
            qWarning() << "ExerciseTourResources::scan: missing id/title in" << path;
            continue;
        }

        entries.append({path, id, title, root.value(QStringLiteral("description")).toString()});
    }

    std::sort(entries.begin(), entries.end(), [](const ExerciseTourResourceEntry &a, const ExerciseTourResourceEntry &b) {
        return a.id < b.id;
    });

    return entries;
}

QVector<ExerciseTourResourceEntry> ExerciseTourResources::mergeUnique(QVector<ExerciseTourResourceEntry> base,
                                                                       const QVector<ExerciseTourResourceEntry> &additional)
{
    for (const ExerciseTourResourceEntry &entry : additional) {
        const bool alreadyPresent = std::any_of(base.cbegin(), base.cend(), [&entry](const ExerciseTourResourceEntry &existing) {
            return existing.id == entry.id;
        });
        if (alreadyPresent) {
            qWarning() << "ExerciseTourResources::mergeUnique: skipping duplicate id" << entry.id << "at" << entry.path;
            continue;
        }
        base.append(entry);
    }

    std::sort(base.begin(), base.end(), [](const ExerciseTourResourceEntry &a, const ExerciseTourResourceEntry &b) {
        return a.id < b.id;
    });

    return base;
}

QString ExerciseTourResources::managedContentDir(const QString &category)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + category;
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
    return path;
}

QStringList ExerciseTourResources::installRelativeCandidates(const QString &category)
{
    const QString appDir = QCoreApplication::applicationDirPath();

    QStringList candidates = {
        appDir + QLatin1Char('/') + category,                       // Windows / dev builds
    };
#ifdef Q_OS_MACOS
    candidates << appDir + QStringLiteral("/../Resources/") + category; // macOS app bundle
#endif
#ifdef Q_OS_LINUX
    candidates << qEnvironmentVariable("APPDIR") + QStringLiteral("/usr/share/wiredpanda/") + category; // AppImage
#endif
#ifdef Q_OS_WASM
    candidates << QStringLiteral("/") + category;                   // WASM virtual filesystem
#endif
    candidates << category;                                          // CWD fallback (development)

    return candidates;
}

QString ExerciseTourResources::installRelativeContentDir(const QString &category)
{
    for (const QString &candidate : installRelativeCandidates(category)) {
        if (!candidate.isEmpty() && QDir(candidate).exists()) {
            return candidate;
        }
    }
    return {};
}

QString ExerciseTourResources::documentsFallbackDir(const QString &category)
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QStringLiteral("/wiRedPanda/") + category;
}

QString ExerciseTourResources::resolveWritableDir(const QStringList &candidates, const QString &documentsFallback)
{
    for (const QString &candidate : candidates) {
        if (candidate.isEmpty()) {
            continue;
        }
        QDir dir(candidate);
        if ((dir.exists() || dir.mkpath(candidate)) && QFileInfo(candidate).isWritable()) {
            return candidate;
        }
    }

    QDir dir(documentsFallback);
    if ((dir.exists() || dir.mkpath(documentsFallback)) && QFileInfo(documentsFallback).isWritable()) {
        return documentsFallback;
    }

    return {};
}

QString ExerciseTourResources::preferredContentDir(const QString &category)
{
    return resolveWritableDir(installRelativeCandidates(category), documentsFallbackDir(category));
}

QVector<ExerciseTourResourceEntry> ExerciseTourResources::discover(const QString &category)
{
    QVector<ExerciseTourResourceEntry> entries = scan(QStringLiteral(":/") + category);

    entries = mergeUnique(entries, scan(managedContentDir(category)));

    const QString installDir = installRelativeContentDir(category);
    if (!installDir.isEmpty()) {
        entries = mergeUnique(entries, scan(installDir));
    }

    const QString docsDir = documentsFallbackDir(category);
    if (QDir(docsDir).exists()) {
        entries = mergeUnique(entries, scan(docsDir));
    }

    return entries;
}

QString ExerciseTourResources::translateFromCatalog(const QString &catalogPath, const QString &key,
                                                      const QString &fallbackEnglish)
{
    QFile file(catalogPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return fallbackEnglish;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return fallbackEnglish;
    }

    QJsonValue node = doc.object();
    const QStringList parts = key.split(QLatin1Char('.'));
    for (const QString &part : parts) {
        if (!node.isObject()) {
            return fallbackEnglish;
        }
        node = node.toObject().value(part);
    }

    return (node.isString() && !node.toString().isEmpty()) ? node.toString() : fallbackEnglish;
}

QString ExerciseTourResources::translate(const QString &key, const QString &fallbackEnglish)
{
    const QString lang = Settings::language();
    if (lang.isEmpty() || lang == QLatin1String("en")) {
        return fallbackEnglish;
    }

    return translateFromCatalog(QStringLiteral(":/i18n/ExerciseTour/%1.json").arg(lang), key, fallbackEnglish);
}
