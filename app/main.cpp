// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "protocol.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("GIBIS-UNIFESP");
    a.setApplicationName("WiredPanda");
    a.setApplicationVersion(APP_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(a.applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "Circuit file to open."));

    QCommandLineOption arduinoFileOption(QStringList() << "a"
                                                       << "arduino-file",
                                         QCoreApplication::translate("main", "Export circuit to <arduino-file>"),
                                         QCoreApplication::translate("main", "arduino file"));
    parser.addOption(arduinoFileOption);

    QCommandLineOption waveformFileOption(QStringList() << "w"
                                                        << "waveform",
                                          QCoreApplication::translate("main", "Export circuit to <waveform> text file"),
                                          QCoreApplication::translate("main", "waveform text file"));
    parser.addOption(waveformFileOption);

    parser.process(a);

    QStringList args = parser.positionalArguments();
    MainWindow w(nullptr, (args.size() > 0 ? QString(args[0]) : QString()));

    QString arduFile = parser.value(arduinoFileOption);
    if (!arduFile.isEmpty()) {
        if (args.size() > 0) {
            w.loadPandaFile(args[0]);
            return !w.exportToArduino(arduFile);
        }
        return 0;
    }
    QString wfFile = parser.value(waveformFileOption);
    if (!wfFile.isEmpty()) {
        if (args.size() > 0) {
            w.loadPandaFile(args[0]);
            return !w.exportToWaveFormFile(wfFile);
        }
        return 0;
    }
    w.show();

    // Initialize TCP Network Protocol
    RemoteProtocol::init(&w);

    if (args.size() > 0) {
        w.loadPandaFile(args[0]);
    }
    return a.exec();
}

/*
 * TODO: Tests for all elements
 * TODO: Create arduino version of all elements
 * TODO: Select some elements, and input wires become input buttons, output wires become leds... Connections are then
 * transferred to the IC's ports.
 */
