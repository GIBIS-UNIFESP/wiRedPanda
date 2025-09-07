#include "application.h"
#include "common.h"

#include <QMessageBox>

#ifdef HAVE_SENTRY
#include "../thirdparty/sentry/include/sentry.h"
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
        // Don't show QMessageBox in MCP mode as it blocks waiting for user interaction
        if (!m_mcpMode) {
            QMessageBox::critical(mainWindow(), tr("Error!"), e.what());
        }
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

bool Application::isInMcpMode() const
{
    return m_mcpMode;
}

void Application::setMcpMode(bool mcpMode)
{
    m_mcpMode = mcpMode;
}
