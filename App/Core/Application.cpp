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

bool Application::notify(QObject *receiver, QEvent *event)
{
    bool done = false;

    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception &e) {
        QMessageBox::critical(mainWindow(), tr("Error!"), e.what());
#ifdef HAVE_SENTRY
        sentry_value_t event_ = sentry_value_new_event();

        // Try to cast to Pandaception to get English message for Sentry
        QString sentryMessage;
        if (const auto *pandaEx = dynamic_cast<const Pandaception*>(&e)) {
            sentryMessage = pandaEx->englishMessage();
        } else {
            sentryMessage = QString::fromStdString(e.what());
        }

        sentry_value_t exc = sentry_value_new_exception("Exception", sentryMessage.toStdString().c_str());
        sentry_value_set_stacktrace(exc, NULL, 0);
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
