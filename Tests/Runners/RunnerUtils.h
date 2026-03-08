// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdio>
#include <functional>
#include <vector>

#include <QObject>
#include <QTest>

#include "App/Core/Application.h"
#include "Tests/Common/TestUtils.h"

struct TestEntry {
    const char *name;
    std::function<QObject *()> create;
};

/**
 * @brief Run a consolidated test suite with optional class filtering.
 *
 * When called from CTest as: executable ClassName [testFunction ...]
 *   - Runs only the named class, passing remaining args to QTest
 *
 * When called with no args:
 *   - Runs all registered classes
 *
 * @param argc  argc from main()
 * @param argv  argv from main()
 * @param tests List of {class name, factory function} entries
 * @return OR of all QTest::qExec() return values (0 = all passed)
 */
inline int runTestSuite(int argc, char **argv, const std::vector<TestEntry> &tests)
{
    TestUtils::setupTestEnvironment();
    Application app(argc, argv);
    TestUtils::configureApp();

    // Global -functions: list all test functions across all classes as ClassName::method()
    if (argc == 2 && QString(argv[1]) == "-functions") {
        for (const auto &entry : tests) {
            QObject *t = entry.create();
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
            delete t;
        }
        return 0;
    }

    // If first arg matches a known class name, run only that class
    if (argc > 1) {
        const QString className = argv[1];
        for (const auto &entry : tests) {
            if (className == entry.name) {
                QObject *t = entry.create();
                // Pass remaining args (after class name) to QTest
                const int result = QTest::qExec(t, argc - 1, argv + 1);
                delete t;
                return result;
            }
        }

        // If argv[1] is not a flag and not a known class name, it's an error
        if (!QString(argv[1]).startsWith('-')) {
            fprintf(stderr, "Unknown test class: %s\n", argv[1]);
            fprintf(stderr, "Available classes:\n");
            for (const auto &entry : tests) {
                fprintf(stderr, "  %s\n", entry.name);
            }
            return 1;
        }
    }

    // No class filter: run all classes in registration order
    int status = 0;
    for (const auto &entry : tests) {
        QObject *t = entry.create();
        status |= QTest::qExec(t, argc, argv);
        delete t;
    }
    return status;
}
