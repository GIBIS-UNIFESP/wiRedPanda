#include "application.h"

#include <QDebug>
#include <QMessageBox>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

bool Application::notify(QObject *receiver, QEvent *event)
{
    bool done;

    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception &e) {
        QMessageBox::critical(nullptr, tr("Error!"), e.what());
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
