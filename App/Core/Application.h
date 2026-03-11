// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QApplication>

#include "App/UI/MainWindow.h"

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

