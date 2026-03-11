#pragma once

#include "mainwindow.h"

#include <QApplication>

#if defined(qApp)
#undef qApp
#endif
#define qApp (qobject_cast<Application *>(QCoreApplication::instance()))

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application() override = default;

    bool notify(QObject *receiver, QEvent *event) override;

    MainWindow *mainWindow() const;
    void setMainWindow(MainWindow *mainWindow);

private:
    Q_DISABLE_COPY(Application)

    MainWindow *m_mainWindow = nullptr;
};
