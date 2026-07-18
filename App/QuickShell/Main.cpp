// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickDialogProvider.h"
#include "App/UI/FileDialogProvider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // appController owns the tab list and every menu action Main.qml binds to; it's exposed
    // as a context property (not a QML_ELEMENT the .qml file instantiates itself) since
    // there's exactly one per running app and it must exist -- with an initial tab already
    // open -- before the QML that reads its properties in Component.onCompleted loads.
    static QuickAppController appController;
    appController.newTab();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appController", &appController);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("QuickShell", "Main");

    // Registered after load (needs the real window to parent dialogs) but before app.exec(),
    // so it's ready before any menu action a real user triggers could call
    // Dialogs::provider()/FileDialogs::provider() -- see DialogProvider.h's and
    // FileDialogProvider.h's contracts.
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    static QuickDialogProvider dialogProvider(window);
    Dialogs::setProvider(&dialogProvider);
    FileDialogs::setProvider(&dialogProvider);

    return app.exec();
}
