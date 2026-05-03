// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Custom QApplication subclass with exception handling and main-window access.
 */

#pragma once

#include <exception>
#include <utility>

#include <QApplication>
#include <QMetaObject>
#include <QPointer>
#include <QString>

#include "App/UI/MainWindow.h"

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
    QString what;            ///< The translated message for QMessageBox display.
    QString englishMessage;  ///< English message for Sentry; equals `what` for non-Pandaception types.
    QString file;            ///< Throw-site file when the exception is a Pandaception, else empty.
    int line = 0;            ///< Throw-site line when the exception is a Pandaception, else 0.
};

/**
 * \class Application
 * \brief Custom QApplication that wraps event dispatch with exception handling.
 *
 * \details Overrides notify() to catch Pandaception and std::exception objects
 * thrown during event processing and display appropriate error dialogs.
 * Also provides typed access to the main window instance.
 */
class Application : public QApplication
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the application with command-line arguments.
     * \param argc Argument count (passed by reference as required by QApplication).
     * \param argv Argument vector.
     */
    Application(int &argc, char **argv);

    /**
     * \brief Returns the application instance cast to Application*.
     * \return Pointer to the Application instance, or nullptr if not available.
     */
    static Application *instance();

    /// Destructor.
    ~Application() override = default;

    // --- Event Handling ---

    /// \reimp Dispatches \a event to \a receiver, catching and reporting exceptions.
    bool notify(QObject *receiver, QEvent *event) override;

    // --- Main Window Access ---

    /// Returns the application's main window.
    MainWindow *mainWindow() const;

    /**
     * \brief Sets the application's main window to \a mainWindow.
     * \param mainWindow Pointer to the MainWindow instance.
     */
    void setMainWindow(MainWindow *mainWindow);

    /// When false, suppresses informational dialogs (e.g. version-mismatch warnings).
    inline static bool interactiveMode = true;

    /// When true, old-format files are automatically backed up and re-saved in the current
    /// format on load. Independent of interactiveMode so tests can enable migration
    /// without triggering any QMessageBox dialogs.
    inline static bool migrationEnabled = true;

    /// Returns true if \a message matches any deny pattern that should be
    /// dropped before being sent to Sentry. Always compiled (independent of
    /// HAVE_SENTRY) so the deny-list policy is unit-testable.
    static bool isSentryDenyMessage(const QString &message);

    // --- Exception handling ---

    /**
     * \brief Centralised exception-reporting handler used by both
     *        `Application::notify` (defence-in-depth on Linux/Windows) and
     *        `Application::guardedSlot` (the macOS-correct catch-in-slot path).
     * \details Shows a QMessageBox::critical when interactiveMode is true, and
     * forwards the event to Sentry as a `handled:1` warning when sentry is
     * compiled in.  Safe to call from a deferred QMetaObject::invokeMethod
     * callback (the exception has finished unwinding by then).
     */
    static void handleException(const ExceptionInfo &info, const QObject *context);

    /**
     * \brief Wraps a slot body in `try/catch` and defers exception reporting
     *        to the next event-loop iteration via a queued `invokeMethod`.
     *
     * \details Per [Qt 6.11 Exception Safety](https://doc.qt.io/qt-6/exceptionsafety.html)
     * and [QTBUG-15197](https://bugreports.qt.io/browse/QTBUG-15197), throwing
     * across Qt's signal-slot dispatch is undefined behaviour; on macOS the
     * unwinder reliably triggers `std::terminate` mid-stack and no upstream
     * `catch` runs.  guardedSlot keeps the catch frame inside the slot itself
     * (below any Qt-internal frame) so the unwinder never crosses a structure
     * that aborts.  The dialog/Sentry forwarding is deferred to the next tick
     * because modal UI from inside a just-unwound event handler is unsafe.
     * See `.claude/SENTRY_TRIAGE.md` §A25.
     *
     * \tparam Body  Invocable callable with no arguments returning anything.
     * \param context  Non-owning pointer used as the receiver hint when the
     *                 deferred `handleException` runs.  Captured via QPointer
     *                 so a destroyed receiver downgrades to nullptr.
     * \param body  The slot body to invoke; any thrown `std::exception` is
     *              caught and reported asynchronously.
     */
    template <typename Body>
    static void guardedSlot(const QObject *context, Body &&body) noexcept
    {
        try {
            std::forward<Body>(body)();
        } catch (const std::exception &e) {
            // Synchronous report: tested deferred (Qt::QueuedConnection) on
            // macOS and the modal QMessageBox::exec() deep inside the queued
            // dispatch hangs (run 25285325668 — 300 s timeout).  The catch
            // here is below the noexcept boundary so std::terminate is not
            // triggered; the modal dialog runs in the slot's frame and
            // returns cleanly before the slot exits.
            handleException(makeExceptionInfo(e), context);
        }
    }

private:
    Q_DISABLE_COPY(Application)

    /// Extracts user-facing and Sentry-side details from a std::exception,
    /// recovering Pandaception-specific fields when applicable.
    static ExceptionInfo makeExceptionInfo(const std::exception &e);

    // --- Members ---

    MainWindow *m_mainWindow = nullptr;
};

