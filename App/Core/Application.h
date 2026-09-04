// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Framework-agnostic application-wide state and exception handling.
 */

#pragma once

#include <exception>
#include <functional>
#include <utility>

#include <QObject>
#include <QString>

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
    QString what;            ///< The translated message for the exception presenter.
    QString englishMessage;  ///< English message for Sentry; equals `what` for non-Pandaception types.
    QString file;            ///< Throw-site file when the exception is a Pandaception, else empty.
    int line = 0;            ///< Throw-site line when the exception is a Pandaception, else 0.
};

/**
 * \class Application
 * \brief Framework-agnostic, purely static application state: the interactive/rendering/
 * migration mode flags every layer reads, and centralised exception reporting.
 *
 * \details Not a QApplication subclass: this class has no notion of QWidget/QMessageBox/event
 * dispatch at all, so a plain Widgets-free domain/test process (or a Qt Quick host) can use it
 * without pulling in Qt Widgets. A real QApplication (WidgetsApplication, Widgets-only) or an
 * equivalent Quick host is expected to override notify() itself for the defence-in-depth
 * exception catch, and to register its own presenter via setExceptionPresenter() so
 * handleException() has a way to actually show the message to the user.
 */
class Application
{
    friend class TestApplication;
    /// notify()-style event-dispatch overrides live on the concrete host (WidgetsApplication is
    /// Widgets' own QApplication subclass; a Quick host would have its own), not here -- they
    /// need makeExceptionInfo() to build the handleException() call this class's own
    /// guardedSlot() makes internally.
    friend class WidgetsApplication;

public:
    /// Registers the bundled font used by element SVG labels (flip-flop / latch pin letters
    /// and the inverted-output overline glyph) so they render identically on every platform.
    /// Portable (QFontDatabase is a Qt Gui, not Qt Widgets, facility) since GraphicElement's
    /// SVG rendering is shared by every host — each host's own startup calls this once, before
    /// any GraphicElement pixmap is built and cached.
    static void registerBundledFonts();

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
    /// without triggering any user-facing dialog.
    inline static bool migrationEnabled = true;

    /// Returns true if \a message matches any deny pattern that should be
    /// dropped before being sent to Sentry. Always compiled (independent of
    /// HAVE_SENTRY) so the deny-list policy is unit-testable.
    static bool isSentryDenyMessage(const QString &message);

    // --- Exception handling ---

    /// Shows \a message to the user via whatever presenter setExceptionPresenter()
    /// registered (a no-op if none has been). \a context is the same receiver hint
    /// handleException() received, forwarded unchanged.
    using ExceptionPresenter = std::function<void(const QString &message, const QObject *context)>;
    /// Registers the presenter used to actually display an exception's message. Call once
    /// at startup; WidgetsApplication registers a QMessageBox-based one, a Quick host would
    /// register its own Dialogs-based one.
    static void setExceptionPresenter(ExceptionPresenter presenter);

    /**
     * \brief Centralised exception-reporting handler used by both a host's own
     *        `notify()` override (defence-in-depth on Linux/Windows) and
     *        `Application::guardedSlot` (the macOS-correct catch-in-slot path).
     * \details Calls the registered presenter when interactiveMode is true, and
     * forwards the event to Sentry as a `handled:1` warning when sentry is
     * compiled in.  Safe to call from a deferred QMetaObject::invokeMethod
     * callback (the exception has finished unwinding by then).
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
     * that aborts.  `handleException` is called directly (not deferred to the
     * next event-loop iteration via a queued `invokeMethod`), using a
     * non-modal presenter so it returns immediately even when interactive:
     * deferring it hangs on macOS, where the modal presenter deep inside the
     * queued dispatch never returns (see the catch block below).
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
            // Synchronous report avoids a macOS hang: a deferred report
            // (Qt::QueuedConnection) leaves the modal presenter stuck deep
            // inside the queued dispatch.  The catch here is below the
            // noexcept boundary so std::terminate is not triggered; the
            // presenter runs in the slot's frame and returns cleanly before
            // the slot exits.
            handleException(makeExceptionInfo(e), context);
        }
    }

private:
    /// Extracts user-facing and Sentry-side details from a std::exception,
    /// recovering Pandaception-specific fields when applicable.
    static ExceptionInfo makeExceptionInfo(const std::exception &e);

    static ExceptionPresenter s_exceptionPresenter;
};
