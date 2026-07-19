// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdio>
#include <functional>
#include <memory>
#include <vector>

#include <QGuiApplication>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

struct QuickTestEntry {
    const char *name;
    std::function<QObject *()> create;
};

/**
 * @brief Run a consolidated Qt Quick-side test suite, mirroring Tests/Runners/RunnerUtils.h's
 * runTestSuite() shape and CLI (executable [ClassName [testFunction ...]] / -functions).
 *
 * Deliberately a separate, self-contained copy rather than a shared header with the Widgets
 * runner: test_wiredpanda_quick constructs a real QGuiApplication (matching
 * App/QuickShell/Main.cpp's own production entry point) instead of the Widgets-only
 * Application/QApplication subclass the Widgets test binary needs -- and this tree is meant
 * to keep working unmodified once Phase 8 deletes the Widgets side entirely.
 */
inline int runQuickTestSuite(int argc, char **argv, const std::vector<QuickTestEntry> &tests)
{
#ifdef Q_OS_LINUX
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    if (!qEnvironmentVariableIsSet("QT_IM_MODULES")) {
        qputenv("QT_IM_MODULES", "none");
    }
#endif
    // Redirect QSettings to a per-process temporary directory, same reasoning as
    // TestUtils::setupTestEnvironment() -- must happen before QGuiApplication/QSettings touch
    // the real config file. Kept alive for the process lifetime (leaked on purpose, mirrors
    // the Widgets runner's identical tradeoff -- the OS reclaims it at exit).
    auto *settingsDir = new QTemporaryDir();
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir->path());
    QStandardPaths::setTestModeEnabled(true);

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("GIBIS-UNIFESP");
    QCoreApplication::setApplicationName("wiRedPanda");

    if (argc == 2 && QString(argv[1]) == "-functions") {
        for (const auto &entry : tests) {
            auto t = std::unique_ptr<QObject>(entry.create());
            const QMetaObject *mo = t->metaObject();
            for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
                const QMetaMethod m = mo->method(i);
                if (m.methodType() != QMetaMethod::Slot || m.access() != QMetaMethod::Private)
                    continue;
                const QByteArray sig = m.methodSignature();
                if (sig == "initTestCase()" || sig == "cleanupTestCase()" ||
                    sig == "init()" || sig == "cleanup()" || sig.startsWith('_'))
                    continue;
                printf("%s::%s\n", entry.name, sig.constData());
            }
        }
        return 0;
    }

    if (argc > 1) {
        const QString className = argv[1];
        for (const auto &entry : tests) {
            if (className == entry.name) {
                auto t = std::unique_ptr<QObject>(entry.create());
                return QTest::qExec(t.get(), argc - 1, argv + 1);
            }
        }
        if (!QString(argv[1]).startsWith('-')) {
            fprintf(stderr, "Unknown test class: %s\n", argv[1]);
            fprintf(stderr, "Available classes:\n");
            for (const auto &entry : tests) {
                fprintf(stderr, "  %s\n", entry.name);
            }
            return 1;
        }
    }

    int status = 0;
    for (const auto &entry : tests) {
        auto t = std::unique_ptr<QObject>(entry.create());
        status |= QTest::qExec(t.get(), argc, argv);
    }
    return status;
}
