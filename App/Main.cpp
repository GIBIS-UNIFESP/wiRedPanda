// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardPaths>

#ifdef HAVE_SENTRY
#include <QScopeGuard>
#endif

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/GlobalProperties.h"
#include "App/RegisterTypes.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"

#ifdef ENABLE_MCP_SERVER
#include "MCP/Server/Core/MCPProcessor.h"
#endif

#ifdef HAVE_SENTRY
#include "thirdparty/sentry/include/sentry.h"
#endif

int main(int argc, char *argv[])
{
#ifdef HAVE_SENTRY
    // Sentry must be initialised before QApplication so that crashes during Qt
    // startup are captured; sentry_close() is called via qScopeGuard at process exit
    // to flush any queued events even if the app exits via an exception or exit().
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://719a4881adf6e678b198bf9aad6b4500@o4508704323469312.ingest.us.sentry.io/4508704326352896");
    // This is also the default-path. For further information and recommendations:
    // https://docs.sentry.io/platforms/native/configuration/options/#database-path
    sentry_options_set_database_path(options, ".sentry-native");
    sentry_options_set_release(options, "wiredpanda@" APP_VERSION);
    sentry_options_set_debug(options, 0);
    sentry_init(options);

    // Make sure everything flushes
    auto sentryClose = qScopeGuard([] { sentry_close(); });
#endif

    // registerTypes() must run before QApplication construction so that Qt's
    // meta-object system knows all custom types (e.g. ElementType, Status) when
    // QVariant, signals/slots, or QDataStream serializers are set up.
    registerTypes();

    // Disable all debug/verbose output at startup; the --verbosity option re-enables
    // it after the command-line parser runs below
    Comment::setVerbosity(-1);

#ifdef ENABLE_MCP_SERVER
    // Early argument parsing for MCP mode detection
    // This must be done before QApplication creation to set Qt platform correctly
    // Full argument validation happens later with QCommandLineParser
    bool mcpMode = false;
    bool mcpGuiMode = false;
    for (int i = 1; i < argc; i++) {
        if (QString(argv[i]) == "--mcp") {
            mcpMode = true;
        }
        if (QString(argv[i]) == "--mcp-gui") {
            mcpGuiMode = true;
        }
    }
#else
    bool mcpMode = false;
    bool mcpGuiMode = false;
#endif

#ifdef Q_OS_LINUX
    // On Linux, CLI-only invocations (codegen, waveform export) must not try to
    // connect to an X11 display — CI runners and headless servers typically don't
    // have one.  Forcing "offscreen" before QApplication avoids a crash.
    bool isCliMode = false;
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "--arduino-file" || arg == "-a" ||
            arg == "--waveform" || arg == "-w" ||
            arg == "--terminal" || arg == "-c" ||
            arg == "--help" || arg == "-h" ||
            arg == "--version" || arg == "-v") {
            isCliMode = true;
            break;
        }
    }

    // Also check if no DISPLAY is available (headless environment)
    if (isCliMode || !qEnvironmentVariableIsSet("DISPLAY")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
#endif

#ifdef Q_OS_WIN
    if (!mcpMode && !mcpGuiMode) {
        // Only attach console for non-MCP modes to preserve stdin/stdout pipes
        FILE *fpstdout = stdout, *fpstderr = stderr;
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            freopen_s(&fpstdout, "CONOUT$", "w", stdout);
            freopen_s(&fpstderr, "CONOUT$", "w", stderr);
        }
    }
#endif

#ifndef Q_OS_WIN
    // Set environment for headless operation if in MCP mode without GUI
    if (mcpMode && !mcpGuiMode) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    Application app(argc, argv);
    app.setOrganizationName("GIBIS-UNIFESP");
    app.setApplicationName("wiRedPanda");
    app.setApplicationVersion(APP_VERSION);
    app.setDesktopFileName("wiredpanda");
    // Fusion style provides a consistent cross-platform look and is required for
    // the custom theme colours defined in ThemeManager to render correctly on all OSes
    app.setStyle("Fusion");
    app.setWindowIcon(QIcon(":/Interface/Toolbar/wpanda.svg"));

#ifdef Q_OS_LINUX
    // When running as an AppImage, the Wayland compositor resolves the window icon
    // by looking up the desktop file for the app_id ("wiredpanda") in the system's
    // XDG_DATA_DIRS — it does not see the AppImage's internal share directory.
    // Self-registering the desktop file and icon to ~/.local/share/ makes the icon
    // visible to the compositor without requiring appimaged or manual installation.
    {
        const QString appImagePath = qEnvironmentVariable("APPIMAGE");
        const QString appDir = qEnvironmentVariable("APPDIR");
        if (!appImagePath.isEmpty() && !appDir.isEmpty()) {
            const QString localShare = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
            const QString appsDir = localShare + "/applications";
            const QString desktopDst = appsDir + "/wiredpanda.desktop";

            // Re-register whenever the AppImage path has changed or the entry is missing.
            bool needsUpdate = !QFile::exists(desktopDst);
            if (!needsUpdate) {
                QFile f(desktopDst);
                if (f.open(QIODevice::ReadOnly))
                    needsUpdate = !QString::fromUtf8(f.readAll()).contains(appImagePath);
            }

            if (needsUpdate) {
                QFile src(appDir + "/usr/share/applications/wiredpanda.desktop");
                if (src.open(QIODevice::ReadOnly)) {
                    QString content = QString::fromUtf8(src.readAll());
                    content.replace(QRegularExpression("^Exec=.*$", QRegularExpression::MultilineOption),
                                    "Exec=\"" + appImagePath + "\" %F");
                    QDir().mkpath(appsDir);
                    QFile dst(desktopDst);
                    if (dst.open(QIODevice::WriteOnly | QIODevice::Truncate))
                        dst.write(content.toUtf8());
                }

                const QString iconsDir = localShare + "/icons/hicolor/scalable/apps";
                QDir().mkpath(iconsDir);
                const QString iconDst = iconsDir + "/wpanda.svg";
                QFile::remove(iconDst);
                QFile::copy(appDir + "/usr/share/icons/hicolor/scalable/apps/wpanda.svg", iconDst);
            }
        }
    }
#endif

    try {
        QCommandLineParser parser;
        parser.setApplicationDescription(app.applicationName());
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("file", QCoreApplication::translate("main", "Circuit file to open."));

        QCommandLineOption verbosityOption(
            {"v2", "verbosity"},
            QCoreApplication::translate("main", "Verbosity level 0 to 5, disabled by default."),
            QCoreApplication::translate("main", "verbosity level"));
        parser.addOption(verbosityOption);

        QCommandLineOption arduinoFileOption(
            {"a", "arduino-file"},
            QCoreApplication::translate("main", "Exports circuit to <arduino-file>."),
            QCoreApplication::translate("main", "arduino file"));
        parser.addOption(arduinoFileOption);

        QCommandLineOption waveformFileOption(
            {"w", "waveform"},
            QCoreApplication::translate("main", "Exports circuit to waveform text file."),
            QCoreApplication::translate("main", "waveform input text file"));
        parser.addOption(waveformFileOption);

        QCommandLineOption terminalFileOption(
            {"c", "terminal"},
            QCoreApplication::translate("main", "Exports circuit to waveform text file, reading input from terminal."));
        parser.addOption(terminalFileOption);

        QCommandLineOption blockTruthTableOption(
            {"btt", "blockTruthTable"},
            QCoreApplication::translate("main", "When used with -c/--terminal, block execution if the circuit contains Truth Tables."));
        parser.addOption(blockTruthTableOption);

#ifdef ENABLE_MCP_SERVER
        QCommandLineOption mcpModeOption(
            "mcp",
            QCoreApplication::translate("main", "Run in MCP (Model Context Protocol) mode for programmatic control."));
        parser.addOption(mcpModeOption);

        QCommandLineOption mcpGuiOption(
            "mcp-gui",
            QCoreApplication::translate("main", "Run MCP mode with a visible GUI window."));
        parser.addOption(mcpGuiOption);
#endif

        parser.process(app);

        if (const QString verbosity = parser.value(verbosityOption); !verbosity.isEmpty()) {
            Comment::setVerbosity(verbosity.toInt());
        }

        QStringList args = parser.positionalArguments();

        // --- Non-interactive batch operations ---
        // Each branch loads the circuit, runs the export, then exits immediately
        // without entering the Qt event loop.  GlobalProperties::verbose is set
        // to false so that error dialogs are suppressed and errors go to stderr only.

        if (const QString arduFile = parser.value(arduinoFileOption); !arduFile.isEmpty()) {
            if (!args.empty()) {
                GlobalProperties::verbose = false;
                MainWindow window;
                window.loadPandaFile(args.at(0));
                window.exportToArduino(arduFile);
            }
            exit(0);
        }

        if (const QString wfFile = parser.value(waveformFileOption); !wfFile.isEmpty()) {
            if (!args.empty()) {
                GlobalProperties::verbose = false;
                MainWindow window;
                window.loadPandaFile(args.at(0));
                window.exportToWaveFormFile(wfFile);
            }
            exit(0);
        }

        if (const bool isTerminal = parser.isSet(terminalFileOption); isTerminal) {
            if (!args.empty()) {
                GlobalProperties::verbose = false;
                MainWindow window;
                window.loadPandaFile(args.at(0));

                // --blockTruthTable allows callers (e.g. automated graders) to
                // reject circuits that contain truth tables, which cannot be
                // reliably evaluated via the terminal waveform interface.
                if (parser.isSet(blockTruthTableOption)) {
                    bool containsTruthTable = false;
                    auto elements = window.currentTab()->scene()->elements();

                    for (const auto *element : elements) {
                        if (element->elementType() == ElementType::TruthTable) {
                            containsTruthTable = true;
                            break;
                        }
                    }

                    if (containsTruthTable) {
                        QTextStream(stderr) << QCoreApplication::translate("main", "Error: Circuit contains Truth Table elements.");
                        exit(1);
                    }
                }

                window.exportToWaveFormTerminal();
            }
            exit(0);
        }

#ifdef ENABLE_MCP_SERVER
        // Handle MCP mode
        if (parser.isSet(mcpModeOption) || parser.isSet(mcpGuiOption)) {
            GlobalProperties::verbose = false;

            auto *window = new MainWindow();
            app.setMainWindow(window);

            // Show window only if GUI mode is requested
            if (parser.isSet(mcpGuiOption)) {
                window->show();
            }

            // Start MCP processor
            MCPProcessor processor(window);
            processor.startProcessing();

            return app.exec();
        }
#endif

        auto *window = new MainWindow();
        app.setMainWindow(window);
        window->show();

        if (!args.empty()) {
            window->loadPandaFile(args.at(0));
        }
    } catch (const std::exception &e) {
        if (GlobalProperties::verbose) {
            QMessageBox::critical(nullptr, QObject::tr("Error!"), e.what());
        }
        exit(1);
    }

    return app.exec();
}
