// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Application.h"

#include <array>

#include <QElapsedTimer>
#include <QMessageBox>

#include "App/Core/Common.h"

#ifdef HAVE_SENTRY
#include "thirdparty/sentry/include/sentry.h"
#endif

namespace {

/// Substrings of throw messages we deliberately drop on the floor.
/// A message belongs here only if it has BOTH of:
///   1. no UX recovery path the user can take when it fires, and
///   2. no actionable signal for us — either the throw is a pure defensive net
///      whose root causes are addressed elsewhere, or it's environmental and
///      not a wiRedPanda bug.
/// Keep this list short. When a new pattern is added, document the Sentry
/// issue ID(s) it suppresses next to the entry so future cleanup can verify
/// the pattern is still load-bearing.
constexpr std::array kSentryDenyPatterns {
    // Defensive nets whose Cluster D root causes were closed in the 5.0.2 cycle
    // (A14/A15/C5/C7/C8). If they fire on a current build it's already too late
    // to act on them: the scene is in a divergent state, the message box has
    // already shown, and the user's next save autosave-rolls a corrupt file.
    "One or more items was not found on the scene",
    "One or more elements was not found on the scene",
    "One or more elements were not found on scene",
};

#ifdef HAVE_SENTRY
/// Deduplicates identical Sentry events within a cooldown window and drops
/// known-noise messages outright.  Returns true if the event should be sent.
bool shouldSendToSentry(const QString &message)
{
    if (Application::isSentryDenyMessage(message)) {
        return false;
    }

    static QString lastMessage;
    static QElapsedTimer timer;

    constexpr qint64 cooldownMs = 5000;

    if (message == lastMessage && timer.isValid() && timer.elapsed() < cooldownMs) {
        return false;
    }

    lastMessage = message;
    timer.start();
    return true;
}
#endif

} // namespace

bool Application::isSentryDenyMessage(const QString &message)
{
    for (const auto *pattern : kSentryDenyPatterns) {
        if (message.contains(QLatin1String(pattern))) {
            return true;
        }
    }
    return false;
}

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

Application *Application::instance()
{
    return qobject_cast<Application *>(QCoreApplication::instance());
}

bool Application::notify(QObject *receiver, QEvent *event)
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
        handleException(makeExceptionInfo(e), receiver);
    }
    return done;
}

ExceptionInfo Application::makeExceptionInfo(const std::exception &e)
{
    ExceptionInfo info;
    info.what = QString::fromUtf8(e.what());
    if (const auto *pandaEx = dynamic_cast<const Pandaception *>(&e)) {
        info.englishMessage = pandaEx->englishMessage();
        const char *f = pandaEx->file();
        info.file = f ? QString::fromUtf8(f) : QString();
        info.line = pandaEx->line();
    } else {
        info.englishMessage = info.what;
    }
    return info;
}

void Application::handleException(const ExceptionInfo &info, const QObject *context)
{
    if (Application::interactiveMode) {
        // Prefer the slot's `this` as the dialog parent (falls back to the
        // Application-stored main window only if context isn't a widget).
        // See guardedSlot's call site for context.
        const QWidget *parent = qobject_cast<const QWidget *>(context);
        if (!parent) {
            auto *self = Application::instance();
            parent = self ? self->mainWindow() : nullptr;
        }

        // Use show() (non-modal) instead of QMessageBox::critical() (modal
        // exec()).  On macOS the modal exec() does not pump Qt timer events
        // reliably from inside Qt's notify dispatch — the polling auto-
        // dismiss in tests gets blocked indefinitely (run 25285904950 +
        // earlier diagnostic runs all showed 300 s hangs).  Non-modal
        // show() lets handleException return immediately; the dialog stays
        // visible, the user clicks OK, and WA_DeleteOnClose cleans up.
        // Slightly less intrusive UX than modal too.
        auto *box = new QMessageBox(QMessageBox::Critical, tr("Error!"),
                                    info.what, QMessageBox::Ok,
                                    const_cast<QWidget *>(parent));
        box->setAttribute(Qt::WA_DeleteOnClose);
        box->show();
    }

#ifdef HAVE_SENTRY
    if (!shouldSendToSentry(info.englishMessage)) {
        return;
    }

    sentry_value_t event_ = sentry_value_new_event();
    sentry_value_set_by_key(event_, "level", sentry_value_new_string("warning"));

    sentry_value_t exc = sentry_value_new_exception(
        "Exception", info.englishMessage.toStdString().c_str());

    sentry_value_t mechanism = sentry_value_new_object();
    sentry_value_set_by_key(mechanism, "type", sentry_value_new_string("generic"));
    sentry_value_set_by_key(mechanism, "handled", sentry_value_new_bool(1));
    if (!info.file.isEmpty()) {
        // Include the throw-site location so Sentry shows where the
        // exception originated rather than the catch block.
        sentry_value_set_by_key(mechanism, "description",
            sentry_value_new_string(
                QStringLiteral("%1:%2").arg(info.file).arg(info.line).toStdString().c_str()));
    }
    sentry_value_set_by_key(exc, "mechanism", mechanism);

    sentry_value_set_stacktrace(exc, NULL, 0);
    sentry_event_add_exception(event_, exc);
    sentry_capture_event(event_);
#endif
}

MainWindow *Application::mainWindow() const
{
    return m_mainWindow;
}

void Application::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;

#ifdef HAVE_SENTRY
    sentry_set_tag("app.interactive", interactiveMode ? "true" : "false");
#endif
}

