// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestNotifyCatch.h"

#include <QAction>
#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include <QTimer>

#include "App/Core/Application.h"
#include "App/Core/Common.h"

namespace {

constexpr int kEventLoopTimeoutMs = 2000;

} // namespace

void TestNotifyCatch::initTestCase()
{
    m_savedInteractiveMode = Application::interactiveMode;
}

void TestNotifyCatch::cleanupTestCase()
{
    Application::interactiveMode = m_savedInteractiveMode;
}

void TestNotifyCatch::guardedSlotCatchesPostedSlotThrow()
{
    Application::interactiveMode = false;

    int slotEntries = 0;
    QAction action;
    QObject::connect(&action, &QAction::triggered, [&slotEntries]() {
        // Not qApp: that macro casts QCoreApplication::instance() to whichever of
        // QCoreApplication/QGuiApplication/QApplication happened to be the last one
        // #include'd anywhere in this translation unit -- test_wiredpanda only ever
        // constructs a QGuiApplication (see QuickRunnerUtils.h), so a stray QApplication
        // cast here is a real UB downcast (UBSan-confirmed). guardedSlot only needs a
        // QObject*, which QCoreApplication::instance() already is.
        Application::guardedSlot(QCoreApplication::instance(), [&slotEntries] {
            ++slotEntries;
            throw PANDACEPTION_LITERAL("notify-catch test throw (no dialog)");
        });
    });

    // Mimic the WIREDPANDA-HQ dispatch path: post a QMetaCall event that
    // invokes the slot via the platform event dispatcher.  On macOS this is
    // delivered through QCocoaEventDispatcherPrivate::postedEventsSourceCallback —
    // historically the path where the old centralised notify() catch silently
    // failed.  guardedSlot keeps the catch inside the slot frame so it works
    // on every platform (see .claude/SENTRY_TRIAGE.md §A25).
    QMetaObject::invokeMethod(&action, "trigger", Qt::QueuedConnection);

    QEventLoop loop;
    QTimer::singleShot(kEventLoopTimeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    // Reaching this assertion means: the slot ran (the throw fired) AND the
    // exception was caught (process did not abort).
    QCOMPARE(slotEntries, 1);
}

void TestNotifyCatch::guardedSlotCatchesPostedSlotThrowInteractive()
{
    Application::interactiveMode = true;

    int slotEntries = 0;
    QAction action;
    QObject::connect(&action, &QAction::triggered, [&slotEntries]() {
        // Not qApp: that macro casts QCoreApplication::instance() to whichever of
        // QCoreApplication/QGuiApplication/QApplication happened to be the last one
        // #include'd anywhere in this translation unit -- test_wiredpanda only ever
        // constructs a QGuiApplication (see QuickRunnerUtils.h), so a stray QApplication
        // cast here is a real UB downcast (UBSan-confirmed). guardedSlot only needs a
        // QObject*, which QCoreApplication::instance() already is.
        Application::guardedSlot(QCoreApplication::instance(), [&slotEntries] {
            ++slotEntries;
            throw PANDACEPTION_LITERAL("notify-catch test throw (interactive)");
        });
    });

    QMetaObject::invokeMethod(&action, "trigger", Qt::QueuedConnection);

    QEventLoop loop;
    QTimer::singleShot(kEventLoopTimeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(slotEntries, 1);

    Application::interactiveMode = false;
}
