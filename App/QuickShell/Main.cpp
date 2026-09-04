// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickDialogProvider.h"
#include "App/UI/FileDialogProvider.h"

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

    // QQmlApplicationEngine below unconditionally attaches its own QQmlFileSelector, applying
    // platform/locale selector-variant checks (e.g. ":/+unix/", ":/+<locale>/") to every QML
    // file and asset load, which shows up as a real hotspot in QResourceRoot::findNode on a
    // large scene. This project has no "+selector/" resource directories anywhere and never
    // will: Exercise/Tour and UI translations both go through their own catalog mechanisms
    // (see CLAUDE.md's i18n section), not per-locale/per-platform asset variants.
    // QT_NO_BUILTIN_SELECTORS must be set before the engine's selector list is first computed
    // (cached for the process's lifetime), so it's set here, before anything QML-related is
    // constructed.
    qputenv("QT_NO_BUILTIN_SELECTORS", "1");

#if defined(ENABLE_MCP_SERVER) && !defined(Q_OS_WIN)
    // Headless MCP mode must not try to connect to a real display -- an automated client
    // driving this over stdin/stdout has no window server to hand it. Mirrors App/Main.cpp's
    // identical early argv scan for the same reason: this must run before QGuiApplication's
    // constructor selects a platform plugin, so QCommandLineParser (which needs a constructed
    // QCoreApplication) is too late. Windows is excluded because its native platform plugin
    // doesn't need a display session the way xcb/cocoa do -- matching App/Main.cpp's own
    // Q_OS_WIN exclusion.
    bool earlyMcpMode = false;
    bool earlyMcpGuiMode = false;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QLatin1String("--mcp")) {
            earlyMcpMode = true;
        } else if (arg == QLatin1String("--mcp-gui")) {
            earlyMcpGuiMode = true;
        }
    }
    if (earlyMcpMode && !earlyMcpGuiMode) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
#endif

    QGuiApplication app(argc, argv);
    Application::registerBundledFonts();

    // A scoped-down mirror of App/Main.cpp's QCommandLineParser handling -- just the
    // positional file argument (none of the export/MCP-mode flags apply to this shell yet).
    // Resolved to an absolute path up front (App/Main.cpp's identical reasoning: any "file does
    // not exist" error should echo back the absolute path the user actually typed, not a bare
    // basename that's confusing once the app's own working directory is involved).
    QCommandLineParser parser;
    parser.setApplicationDescription(app.applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "Circuit file to open."));

#ifdef ENABLE_MCP_SERVER
    // Mirrors App/Main.cpp's identical pair -- --mcp-gui exists there only to distinguish a
    // shown-but-headless MainWindow from a never-shown one; wiredpanda's ApplicationWindow
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

    // ThemeManager::instance() is a lazily-constructed Meyer's singleton whose constructor
    // asserts it's running on the GUI thread. CanvasItem::updatePaintNode()/appearanceKeyFor()
    // read ThemeManager::attributes() on Qt Quick's scene graph render thread (a real, separate
    // thread once a scene is rich enough to trigger the threaded render loop) -- forcing
    // construction here, on the GUI thread, before the engine loads any QML, guarantees the
    // render thread can never be the one that constructs it. Mirrors App/Main.cpp's identical
    // early re-apply for the same "construct on the GUI thread first" reason.
    ThemeManager::setTheme(ThemeManager::theme());

    // appController owns the tab list and every menu action Main.qml binds to; it's exposed
    // as the AppController QML singleton (AppControllerForeign, in QuickAppController.h) since
    // there's exactly one per running app and it must exist -- with an initial tab already
    // open -- before the QML that reads its properties in Component.onCompleted loads. A
    // context property would also satisfy that ordering constraint, but is invisible to
    // qmllint/the QML Language Server; the singleton isn't.
    //
    // Deliberately NOT `static`: a function-local static's destructor runs at real process
    // exit (via atexit, inside __run_exit_handlers), which is AFTER main()'s own automatic
    // (stack) locals -- including `app` itself -- have already been destroyed. QuickWorkSpace's
    // destructor calls Settings::autosaveFiles()/setAutosaveFiles(), which needs a live
    // QCoreApplication; with `static` storage, appController (and its whole tab list) is
    // torn down only after `app` is already gone, producing a SIGSEGV inside
    // ~QuickWorkSpace(). A plain automatic variable is destroyed in the normal, correct
    // reverse-declaration order at the end of main()'s own scope, before `app` is destroyed.
    QuickAppController appController;
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
    // FileDialogProvider.h's contracts. Also deliberately not `static`, for the same
    // destruction-order reason as appController above; declared after `engine` so it's
    // destroyed before `engine` (its QQmlComponent members reference the engine).
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    QuickDialogProvider dialogProvider(window);
    Dialogs::setProvider(&dialogProvider);
    FileDialogs::setProvider(&dialogProvider);

    // Loaded after the dialog providers are registered so a load failure's error path (once
    // QuickWorkspaceManager/QuickWorkSpace surfaces one through Dialogs::provider()) has a
    // real provider to use, mirroring App/Main.cpp's own call-after-show() ordering. Mirrors
    // MainWindow's own behavior of always having an initial empty tab already open underneath
    // the loaded one -- not a Quick-specific quirk to special-case around.
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

    // Mirrors App/UI/MainWindow::show()'s own call-after-show() ordering; internally gated
    // (Application::interactiveMode, "already checked today", auto-checks disabled) so this is
    // always safe to call unconditionally here.
    appController.updateController()->checkForUpdates();

    return app.exec();
}
