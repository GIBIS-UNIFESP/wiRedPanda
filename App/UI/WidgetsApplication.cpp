// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WidgetsApplication.h"

#include <QFontDatabase>
#include <QMessageBox>
#include <QWidget>

#include "App/Core/Application.h"

WidgetsApplication::WidgetsApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    // Register the bundled font used by element SVG labels (flip-flop / latch pin letters and the
    // inverted-output overline glyph) so they render identically on every platform — before any
    // GraphicElement pixmap is built and cached. Both the desktop entry and the test runner
    // construct WidgetsApplication before any element exists, so this single spot covers both.
    if (QFontDatabase::addApplicationFont(QStringLiteral(":/Fonts/NotoSans-Regular.ttf")) == -1) {
        qWarning() << "Failed to register bundled font: NotoSans-Regular.ttf";
    }

    // Use show() (non-modal) instead of QMessageBox::critical() (modal exec()). On macOS the
    // modal exec() does not pump Qt timer events reliably from inside Qt's notify dispatch —
    // the polling auto-dismiss in tests gets blocked indefinitely (run 25285904950 + earlier
    // diagnostic runs all showed 300 s hangs). Non-modal show() lets handleException() return
    // immediately; the dialog stays visible, the user clicks OK, and WA_DeleteOnClose cleans up.
    Application::setExceptionPresenter([](const ExceptionInfo &info, const QObject *context) {
        // Prefer the slot's `this` as the dialog parent, falling back to whatever top-level
        // window currently has focus if context isn't a widget.
        const QWidget *parent = qobject_cast<const QWidget *>(context);
        if (!parent) {
            parent = QApplication::activeWindow();
        }
        auto *box = new QMessageBox(QMessageBox::Critical, tr("Error!"),
                                    info.what, QMessageBox::Ok,
                                    const_cast<QWidget *>(parent));
        box->setAttribute(Qt::WA_DeleteOnClose);
        box->show();
    });
}

bool WidgetsApplication::notify(QObject *receiver, QEvent *event)
{
    // Defence-in-depth backstop for exceptions escaping Qt event handlers on
    // Linux/Windows.  This catch is structurally UNREACHABLE on macOS for
    // exceptions thrown from queued slots (Qt 6.11 Exception Safety doc +
    // QTBUG-15197) — slots that may throw must wrap their body in
    // Application::guardedSlot.  See .claude/SENTRY_TRIAGE.md §A25.
    bool done = false;
    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception &e) {
        Application::handleException(Application::makeExceptionInfo(e), receiver);
    }
    return done;
}
