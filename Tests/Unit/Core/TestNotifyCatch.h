// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestNotifyCatch
 * \brief Verifies that exceptions thrown from slots invoked through the
 *        platform event dispatcher's posted-events callback propagate to
 *        Application::notify's try/catch.
 *
 * \details Regression cover for WIREDPANDA-HQ on macOS arm64, where a
 * PANDACEPTION thrown from BewavedDolphin::loadElements() escaped
 * Application::notify and produced a SIGABRT minidump. On macOS the slot
 * was invoked via QCocoaEventDispatcherPrivate::postedEventsSourceCallback;
 * these tests reproduce that dispatch path with a posted QMetaCall.
 *
 * If the catch is broken on a platform, the test process aborts and CTest
 * reports the failure — that is the diagnostic signal we want.
 */
class TestNotifyCatch : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    /// Pure catch test: throw from a posted-event slot, no dialog involved.
    /// Isolates whether Application::notify's catch fires at all on the platform.
    void postedSlotThrowIsCaught();

    /// Same dispatch path, but with interactiveMode=true so the catch handler
    /// also calls QMessageBox::critical (auto-dismissed). Tests whether the
    /// dialog from inside the catch — invoked while still inside a CFRunLoop
    /// source callback on macOS — is itself the killer (hypothesis A).
    void postedSlotThrowIsCaughtWithDialog();

private:
    bool m_savedInteractiveMode = true;
};

