// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestNotifyCatch.h"

#include <QAction>
#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QObject>
#include <QTimer>

#include "App/Core/Application.h"
#include "App/Core/Common.h"

namespace {

constexpr int kEventLoopTimeoutMs = 2000;

/// Polls for any visible QMessageBox and dismisses it. Used to keep the
/// dialog-variant test from hanging on a modal critical box opened from
/// inside Application::notify's catch handler.
void startMessageBoxAutoDismiss(QTimer &timer)
{
    timer.setInterval(50);
    QObject::connect(&timer, &QTimer::timeout, []() {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *w : widgets) {
            if (auto *box = qobject_cast<QMessageBox *>(w)) {
                if (box->isVisible()) {
                    box->done(QMessageBox::Ok);
                }
            }
        }
    });
    timer.start();
}

} // namespace

void TestNotifyCatch::initTestCase()
{
    m_savedInteractiveMode = Application::interactiveMode;
}

void TestNotifyCatch::cleanupTestCase()
{
    Application::interactiveMode = m_savedInteractiveMode;
}

void TestNotifyCatch::postedSlotThrowIsCaught()
{
    Application::interactiveMode = false;

    int slotEntries = 0;
    QAction action;
    QObject::connect(&action, &QAction::triggered, [&slotEntries]() {
        ++slotEntries;
        throw PANDACEPTION("notify-catch test throw (no dialog)");
    });

    // Mimic the WIREDPANDA-HQ dispatch path: post a QMetaCall event that
    // invokes the slot via QApplication::notify. On macOS this is delivered
    // through QCocoaEventDispatcherPrivate::postedEventsSourceCallback.
    QMetaObject::invokeMethod(&action, "trigger", Qt::QueuedConnection);

    QEventLoop loop;
    QTimer::singleShot(kEventLoopTimeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    // Reaching this assertion means: the slot ran (the throw fired) AND the
    // exception was caught by Application::notify (process did not abort).
    QCOMPARE(slotEntries, 1);
}

void TestNotifyCatch::postedSlotThrowIsCaughtWithDialog()
{
    Application::interactiveMode = true;

    QTimer dismiss;
    startMessageBoxAutoDismiss(dismiss);

    int slotEntries = 0;
    QAction action;
    QObject::connect(&action, &QAction::triggered, [&slotEntries]() {
        ++slotEntries;
        throw PANDACEPTION("notify-catch test throw (with dialog)");
    });

    QMetaObject::invokeMethod(&action, "trigger", Qt::QueuedConnection);

    QEventLoop loop;
    QTimer::singleShot(kEventLoopTimeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(slotEntries, 1);

    Application::interactiveMode = false;
}

