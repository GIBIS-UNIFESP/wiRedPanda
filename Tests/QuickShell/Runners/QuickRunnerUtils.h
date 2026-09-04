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

#include "App/Core/Application.h"

struct QuickTestEntry {
    const char *name;
    std::function<QObject *()> create;
};

/**
 * \brief Runs a consolidated Quick-side test suite, mirroring Tests/Runners/RunnerUtils.h's
 * runTestSuite() shape and CLI (executable [ClassName [testFunction ...]] / -functions).
 *
 * Deliberately a separate, self-contained copy rather than a shared header with the Widgets
 * runner: test_wiredpanda constructs a plain QGuiApplication (there is no Quick-specific
 * QApplication subclass yet -- App/UI/WidgetsApplication.h is the Widgets-only one) instead of
 * the Widgets test binary's WidgetsApplication, and this tree is meant to keep working
 * unmodified once the Widgets side is eventually deleted.
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
    // Redirect QSettings to a per-process temporary directory, same reasoning and same static-
    // storage-duration pattern as TestUtils::setupTestEnvironment() -- must happen before
    // QGuiApplication/QSettings touch the real config file. Static so it stays alive for the
    // process lifetime while still being destroyed at normal static-teardown time, well after
    // any test logic runs.
    static QTemporaryDir settingsDir;
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
    QStandardPaths::setTestModeEnabled(true);

    // Same reasoning as TestUtils::setupTestEnvironment(): suppresses UI dialogs, and --
    // load-bearing for Simulation::update(), whose visual-refresh throttle only engages when
    // Application::interactiveMode is true -- otherwise a test driving Simulation::update()
    // synchronously, with no real wall-clock time between ticks, can see stale port statuses
    // for many ticks.
    Application::interactiveMode = false;
    // Also matches TestUtils::setupTestEnvironment(): preserves backward-compatibility test
    // fixtures (old-format .panda files) as-is instead of silently auto-migrating them on load.
    Application::migrationEnabled = false;

    // TestQuickDialogProvider drives QuickFileDialog.qml's FileDialog directly -- setting
    // "selectedFile"/invoking "accept" on the QML object -- which only reaches the dialog's
    // real state when Qt is using its own QML content instead of a real platform-native
    // dialog. QQuickAbstractDialog::useNativeDialog() checks this attribute first, before
    // even asking the platform theme, so this reliably forces the QML fallback on every
    // platform.
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("GIBIS-UNIFESP");
    QCoreApplication::setApplicationName("wiRedPanda");
    // Deterministic label metrics across every CI platform -- see labelFont()'s own doc
    // comment (App/Element/GraphicElement.cpp) for why this matters for layout-sensitive
    // tests like TestICFixtureLayout.
    Application::registerBundledFonts();

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
