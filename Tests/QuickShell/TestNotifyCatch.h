// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestNotifyCatch
 * \brief Verifies that Application::guardedSlot catches exceptions thrown from
 *        slots invoked through the platform event dispatcher's posted-events
 *        callback.
 *
 * \details Regression cover for WIREDPANDA-HQ.  Originally written against
 * Application::notify's try/catch, which was the historical centralised catch
 * site — but Qt 6.11 Exception Safety doc and the ~14 y old QTBUG-15197
 * established that this catch is structurally unreachable on macOS for queued
 * slot dispatch.  The fix moved the catch into the slot frame via
 * `Application::guardedSlot(this, [this] { … })`, which sits below Qt's macOS
 * unwind boundary.  wiredpanda_quick's plain QGuiApplication has no notify()
 * override at all (unlike wiredpanda's WidgetsApplication), so guardedSlot is
 * the ONLY exception-safety mechanism on this side, not just the macOS-correct
 * one. These tests assert the contract: a slot that wraps its body in
 * guardedSlot returns cleanly when its body throws and the caught exception is
 * reported via Application::handleException. See `.claude/SENTRY_TRIAGE.md`
 * §A25 for the full investigation.
 */
class TestNotifyCatch : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    /// Pure catch test: throw from a posted-event slot wrapped in
    /// Application::guardedSlot, no dialog involved.  Asserts that the slot
    /// returns and the test process does not abort.
    void guardedSlotCatchesPostedSlotThrow();

    /// Same dispatch path with interactiveMode=true so handleException invokes
    /// the registered Dialogs::provider() presenter, mirroring App/QuickShell/
    /// Main.cpp's real registration.
    void guardedSlotCatchesPostedSlotThrowWithDialog();

private:
    bool m_savedInteractiveMode = true;
};
