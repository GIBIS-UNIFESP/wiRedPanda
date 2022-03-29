#pragma once

#include <QApplication>

#if defined(qApp)
#undef qApp
#endif
#define qApp (dynamic_cast<Application *>(QCoreApplication::instance()))

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application() = default;

    bool notify(QObject *receiver, QEvent *event);
};

