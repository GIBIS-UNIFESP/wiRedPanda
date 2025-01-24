// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "common.h"
#include "globalproperties.h"
#include "mainwindow.h"

#include <QCommandLineParser>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    Comment::setVerbosity(-1);

#ifdef Q_OS_WIN
    FILE *fpstdout = stdout, *fpstderr = stderr;
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen_s(&fpstdout, "CONOUT$", "w", stdout);
        freopen_s(&fpstderr, "CONOUT$", "w", stderr);
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
    app.setStyle("Fusion");
    app.setWindowIcon(QIcon(":/toolbar/wpanda.svg"));

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
            QCoreApplication::translate("main", "Exports circuit to <arduino-file>"),
            QCoreApplication::translate("main", "arduino file"));
        parser.addOption(arduinoFileOption);

        QCommandLineOption waveformFileOption(
            {"w", "waveform"},
            QCoreApplication::translate("main", "Exports circuit to waveform text file"),
            QCoreApplication::translate("main", "waveform input text file"));
        parser.addOption(waveformFileOption);

        QCommandLineOption terminalFileOption(
            {"c", "terminal"},
            QCoreApplication::translate("main", "Exports circuit to waveform text file, reading input from terminal"));
        parser.addOption(terminalFileOption);

        parser.process(app);

        if (const QString verbosity = parser.value(verbosityOption); !verbosity.isEmpty()) {
            Comment::setVerbosity(verbosity.toInt());
        }

        QStringList args = parser.positionalArguments();

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
                window.exportToWaveFormTerminal();
            }
            exit(0);
        }

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

// TODO: Tests for all elements
// TODO: Create arduino version of all elements
// TODO: Select some elements, and input wires become input buttons, output wires become leds...
// ...Connections are then transferred to the IC's ports.
// TODO: ambiguous shortcut overloads (ctrl+y)
