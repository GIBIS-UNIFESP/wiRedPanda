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
    // Overriding notify() is the only reliable way to catch exceptions thrown inside
    // Qt event handlers, because Qt does not propagate C++ exceptions through its own
    // event dispatch loop — they would terminate the program with std::terminate() instead.
    bool done = false;

    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception &e) {
        if (Application::interactiveMode) {
            QMessageBox::critical(mainWindow(), tr("Error!"), e.what());
        }
#ifdef HAVE_SENTRY
        // Pandaception carries a separate English message for Sentry so that
        // crash reports are readable even when the UI is shown in another locale.
        // It also stores the throw-site file/line for accurate stack location.
        QString sentryMessage;
        const char *throwFile = nullptr;
        int throwLine = 0;

        if (const auto *pandaEx = dynamic_cast<const Pandaception*>(&e)) {
            sentryMessage = pandaEx->englishMessage();
            throwFile = pandaEx->file();
            throwLine = pandaEx->line();
        } else {
            sentryMessage = QString::fromStdString(e.what());
        }

        if (shouldSendToSentry(sentryMessage)) {
            sentry_value_t event_ = sentry_value_new_event();
            sentry_value_set_by_key(event_, "level", sentry_value_new_string("warning"));

            sentry_value_t exc = sentry_value_new_exception("Exception", sentryMessage.toStdString().c_str());

            sentry_value_t mechanism = sentry_value_new_object();
            sentry_value_set_by_key(mechanism, "type", sentry_value_new_string("generic"));
            sentry_value_set_by_key(mechanism, "handled", sentry_value_new_bool(1));
            if (throwFile) {
                // Include the throw-site location so Sentry shows where the
                // exception originated rather than the catch block.
                sentry_value_set_by_key(mechanism, "description",
                    sentry_value_new_string(QStringLiteral("%1:%2").arg(throwFile).arg(throwLine).toStdString().c_str()));
            }
            sentry_value_set_by_key(exc, "mechanism", mechanism);

            sentry_value_set_stacktrace(exc, NULL, 0);
            sentry_event_add_exception(event_, exc);
            sentry_capture_event(event_);
        }
#endif
    }

    return done;
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

