// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Application-wide runtime flags and exception-safety helpers, shared by both apps.
 */

#pragma once

#include <exception>
#include <functional>
#include <utility>

#include <QString>

class QObject;

/**
 * \struct ExceptionInfo
 * \brief Value-captured exception details safe to forward across an event-loop
 *        boundary.
 *
 * \details `std::exception` references are not safe to capture in a deferred
 * `QMetaObject::invokeMethod` callback — the exception object is destroyed
 * when the catch block exits.  ExceptionInfo holds copies of the bits we
 * need so the deferred `Application::handleException` callback can run after
 * the throw has finished unwinding.
 */
struct ExceptionInfo
{
    QString what;            ///< The translated message shown to the user.
    QString englishMessage;  ///< English message for Sentry; equals `what` for non-Pandaception types.
    QString file;            ///< Throw-site file when the exception is a Pandaception, else empty.
    int line = 0;            ///< Throw-site line when the exception is a Pandaception, else 0.
};

/**
 * \class Application
 * \brief Cross-app runtime flags and exception-safety helpers.
 *
 * \details Framework-agnostic: no longer a `QApplication` subclass (that's
 * `App/UI/WidgetsApplication.h`, the real application object `wiredpanda`'s
 * `App/Main.cpp` constructs — `wiredpanda_quick` uses a plain `QGuiApplication`).
 * Both apps call these statics directly. Presenting an exception to the user is done
 * through a pluggable presenter (setExceptionPresenter()) each app registers once at
 * startup, mirroring `FileDialogs::setDefaultProvider()`'s pattern: `wiredpanda` registers
 * a real `QMessageBox`; `wiredpanda_quick` registers `Dialogs::provider()->choice()`.
 */
class Application
{
public:
    /// When false, suppresses informational dialogs (e.g. version-mismatch warnings).
    /// Stays false in BOTH MCP modes: an automated session can't dismiss a dialog.
    /// Visual concerns are governed by renderingEnabled instead.
    inline static bool interactiveMode = true;

    /// When false, skips wire-geometry construction (Connection::updatePath)
    /// as a throughput optimization for contexts that never paint or export:
    /// unit tests and fuzz harnesses. Must stay true whenever anything can be
    /// rendered — including headless --mcp, whose export_image paints the
    /// scene off-screen.
    inline static bool renderingEnabled = true;

    /// When true, old-format files are automatically backed up and re-saved in the current
    /// format on load. Independent of interactiveMode so tests can enable migration
    /// without triggering any dialogs.
    inline static bool migrationEnabled = true;

    /// Returns true if \a message matches any deny pattern that should be
    /// dropped before being sent to Sentry. Always compiled (independent of
    /// HAVE_SENTRY) so the deny-list policy is unit-testable.
    static bool isSentryDenyMessage(const QString &message);

    // --- Exception handling ---

    /// Registers the callback handleException() invokes (when interactiveMode is true) to
    /// present an exception to the user. Each app calls this once at startup with its own
    /// concrete presentation mechanism; an app that never registers one simply skips
    /// presentation (Sentry reporting below still happens).
    static void setExceptionPresenter(std::function<void(const ExceptionInfo &, const QObject *)> presenter);

    /**
     * \brief Centralised exception-reporting handler used by both each app's `notify()`
     *        override (defence-in-depth on Linux/Windows) and `Application::guardedSlot`
     *        (the macOS-correct catch-in-slot path).
     * \details Invokes the registered exception presenter (see setExceptionPresenter()) when
     * interactiveMode is true, and forwards the event to Sentry as a `handled:1` warning when
     * sentry is compiled in. Safe to call from a deferred `QMetaObject::invokeMethod` callback
     * (the exception has finished unwinding by then).
     */
    static void handleException(const ExceptionInfo &info, const QObject *context);

    /**
     * \brief Wraps a slot body in `try/catch` and reports any exception
     *        synchronously, inside the slot's own stack frame.
     *
     * \details Per [Qt 6.11 Exception Safety](https://doc.qt.io/qt-6/exceptionsafety.html)
     * and [QTBUG-15197](https://bugreports.qt.io/browse/QTBUG-15197), throwing
     * across Qt's signal-slot dispatch is undefined behaviour; on macOS the
     * unwinder reliably triggers `std::terminate` mid-stack and no upstream
     * `catch` runs.  guardedSlot keeps the catch frame inside the slot itself
     * (below any Qt-internal frame) so the unwinder never crosses a structure
     * that aborts.  Reporting was originally deferred to the next event-loop
     * iteration via a queued `invokeMethod`, but that hangs on macOS — a modal
     * dialog deep inside the queued dispatch never returns (see the catch block
     * below and `.claude/SENTRY_TRIAGE.md` §A25). `handleException` is called
     * directly instead, using a non-modal presentation so it returns immediately
     * even when interactive.
     *
     * \tparam Body  Invocable callable with no arguments returning anything.
     * \param context  Non-owning pointer used as the receiver hint passed to
     *                 `handleException`.
     * \param body  The slot body to invoke; any thrown `std::exception` is
     *              caught and reported before this function returns.
     */
    template <typename Body>
    static void guardedSlot(const QObject *context, Body &&body) noexcept
    {
        try {
            std::forward<Body>(body)();
        } catch (const std::exception &e) {
            // Synchronous report: tested deferred (Qt::QueuedConnection) on
            // macOS and a modal dialog deep inside the queued dispatch hangs
            // (run 25285325668 — 300 s timeout). The catch here is below the
            // noexcept boundary so std::terminate is not triggered; the dialog
            // runs in the slot's frame and returns cleanly before the slot exits.
            handleException(makeExceptionInfo(e), context);
        }
    }

    /// Extracts user-facing and Sentry-side details from a std::exception,
    /// recovering Pandaception-specific fields when applicable. Public so each app's own
    /// `notify()` override (a member of a different class, App/UI/WidgetsApplication.h) can
    /// build an ExceptionInfo the same way guardedSlot() does above.
    static ExceptionInfo makeExceptionInfo(const std::exception &e);
};
