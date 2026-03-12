// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Application.h"

#include <QMessageBox>

#include "App/Core/Common.h"

#ifdef HAVE_SENTRY
#include "thirdparty/sentry/include/sentry.h"
#endif

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
        sentry_value_t event_ = sentry_value_new_event();

        // Pandaception carries a separate English message for Sentry so that
        // crash reports are readable even when the UI is shown in another locale.
        QString sentryMessage;
        if (const auto *pandaEx = dynamic_cast<const Pandaception*>(&e)) {
            sentryMessage = pandaEx->englishMessage();
        } else {
            sentryMessage = QString::fromStdString(e.what());
        }

        sentry_value_t exc = sentry_value_new_exception("Exception", sentryMessage.toStdString().c_str());
        sentry_value_set_stacktrace(exc, NULL, 0); // NULL/0 → capture current stack automatically
        sentry_event_add_exception(event_, exc);
        sentry_capture_event(event_);
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
}
