// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/FileDialogProvider.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickDialogProvider.h"

#ifdef ENABLE_MCP_SERVER
#include "MCP/Server/Core/QuickMCPProcessor.h"
#endif

int main(int argc, char *argv[])
{
    // Disable all debug/verbose output at startup -- mirrors App/Main.cpp's identical call,
    // without which Qt's custom logging categories (qCDebug(zero)/qCDebug(two)/etc., used
    // throughout Simulation.cpp and ElementFactory registration) default to enabled and spam
    // the console on every tab/CanvasItem construction.
    Comment::setVerbosity(-1);

    QGuiApplication app(argc, argv);

    // A scoped-down mirror of App/Main.cpp's QCommandLineParser handling -- just the
    // positional file argument (none of the export/MCP-mode flags apply to this shell yet).
    // Resolved to an absolute path up front (App/Main.cpp:356's identical reasoning: any
    // "file does not exist" error should echo back the absolute path the user actually typed,
    // not a bare basename that's confusing once the app's own working directory is involved).
    QCommandLineParser parser;
    parser.setApplicationDescription(app.applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "Circuit file to open."));

#ifdef ENABLE_MCP_SERVER
    // Mirrors App/Main.cpp's identical pair -- --mcp-gui exists there only to distinguish a
    // shown-but-headless MainWindow from a never-shown one; wiredpanda_quick's ApplicationWindow
    // hardcodes visible: true (Main.qml), so both flags behave identically here. Kept as two
    // options (not collapsed into one) so existing MCP client tooling that passes either flag
    // keeps working unmodified.
    QCommandLineOption mcpModeOption("mcp",
        QCoreApplication::translate("main", "Run in MCP (Model Context Protocol) mode for programmatic control."));
    parser.addOption(mcpModeOption);
    QCommandLineOption mcpGuiOption("mcp-gui",
        QCoreApplication::translate("main", "Run MCP mode with a visible GUI window."));
    parser.addOption(mcpGuiOption);
#endif

    parser.process(app);
    const QStringList positionalArgs = parser.positionalArguments();
    const QString inputFile = positionalArgs.isEmpty()
        ? QString()
        : QDir::current().absoluteFilePath(positionalArgs.at(0));

#ifdef ENABLE_MCP_SERVER
    const bool mcpMode = parser.isSet(mcpModeOption) || parser.isSet(mcpGuiOption);
    if (mcpMode) {
        Application::interactiveMode = false;
    }
#endif

    // appController owns the tab list and every menu action Main.qml binds to; it's exposed
    // as the AppController QML singleton (AppControllerForeign, in QuickAppController.h) since
    // there's exactly one per running app and it must exist -- with an initial tab already
    // open -- before the QML that reads its properties in Component.onCompleted loads. A
    // context property would also satisfy that ordering constraint, but is invisible to
    // qmllint/the QML Language Server; the singleton isn't.
    static QuickAppController appController;
    appController.newTab();
    AppControllerForeign::s_instance = &appController;

    QQmlApplicationEngine engine;
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
    FileDialogs::setDefaultProvider(&dialogProvider);

    // Mirrors MainWindow::show()'s single checkForUpdates() call -- internally gated on
    // Application::interactiveMode, so this is a safe no-op in MCP mode too.
    appController.updateController()->checkForUpdates();

    // Loaded after the dialog providers are registered so a load failure's error path (once
    // QuickWorkspaceManager/QuickWorkSpace surfaces one through Dialogs::provider()) has a
    // real provider to use, mirroring App/Main.cpp's own call-after-show() ordering. Mirrors
    // MainWindow's own behavior of always having an initial empty tab already open underneath
    // the loaded one (App/UI/MainWindow.cpp:147's unconditional createNewTab()) -- not a
    // Quick-specific quirk to special-case around.
#ifdef ENABLE_MCP_SERVER
    if (mcpMode) {
        // MCP mode: skip positional-file loading (an MCP client loads circuits via its own
        // load_circuit call, mirroring App/Main.cpp's identical early-return before its own
        // positional-file handling) and start the MCP stdin/stdout processor instead.
        QuickMCPProcessor processor(&appController);
        processor.startProcessing();
        return app.exec();
    }
#endif

    if (!inputFile.isEmpty()) {
        appController.openRecentFile(inputFile);
    }

    return app.exec();
}
