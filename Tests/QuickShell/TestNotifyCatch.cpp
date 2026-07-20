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
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "Tests/QuickShell/StubDialogProvider.h"

namespace {

constexpr int kEventLoopTimeoutMs = 2000;

} // namespace

void TestNotifyCatch::initTestCase()
{
    m_savedInteractiveMode = Application::interactiveMode;

    // Mirrors App/QuickShell/Main.cpp's own registration -- routes through
    // Dialogs::provider(), which is "never null" (falls back to a real
    // QuickDialogProvider), so this is safe to leave installed for the whole
    // test binary, exactly like the real app's one-time startup registration.
    Application::setExceptionPresenter([](const ExceptionInfo &info, const QObject *) {
        Dialogs::provider()->choice(QObject::tr("Error!"), info.what, {DialogButton::Ok}, DialogButton::Ok);
    });
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
        Application::guardedSlot(qApp, [&slotEntries] {
            ++slotEntries;
            throw PANDACEPTION_LITERAL("notify-catch test throw (no dialog)");
        });
    });

    // Mimic the WIREDPANDA-HQ dispatch path: post a QMetaCall event that
    // invokes the slot via the platform event dispatcher, not a direct call.
    // guardedSlot keeps the catch inside the slot frame so it works on every
    // platform (see .claude/SENTRY_TRIAGE.md §A25).
    QMetaObject::invokeMethod(&action, "trigger", Qt::QueuedConnection);

    QEventLoop loop;
    QTimer::singleShot(kEventLoopTimeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    // Reaching this assertion means: the slot ran (the throw fired) AND the
    // exception was caught (process did not abort).
    QCOMPARE(slotEntries, 1);
}

void TestNotifyCatch::guardedSlotCatchesPostedSlotThrowWithDialog()
{
    Application::interactiveMode = true;

    ScopedDialogStub dialogStub;

    int slotEntries = 0;
    QAction action;
    QObject::connect(&action, &QAction::triggered, [&slotEntries]() {
        Application::guardedSlot(qApp, [&slotEntries] {
            ++slotEntries;
            throw PANDACEPTION_LITERAL("notify-catch test throw (with dialog)");
        });
    });

    QMetaObject::invokeMethod(&action, "trigger", Qt::QueuedConnection);

    QEventLoop loop;
    QTimer::singleShot(kEventLoopTimeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(slotEntries, 1);
    QCOMPARE(dialogStub.stub.choiceCallCount, 1);

    Application::interactiveMode = false;
}
