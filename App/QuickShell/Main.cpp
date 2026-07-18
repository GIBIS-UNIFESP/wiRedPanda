// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickDialogProvider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("QuickShell", "Main");

    // Registered before app.exec() so it's ready before any controller code (ported in later
    // Phase 4 sub-steps) could call Dialogs::provider() -- see DialogProvider.h's contract.
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    static QuickDialogProvider dialogProvider(window);
    Dialogs::setProvider(&dialogProvider);

    return app.exec();
}
