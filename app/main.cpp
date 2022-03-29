// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "common.h"
#include "mainwindow.h"

#include <QCommandLineParser>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    Comment::setVerbosity(1);

    Application a(argc, argv);
    a.setOrganizationName("GIBIS-UNIFESP");
    a.setApplicationName("WiRedPanda");
    a.setApplicationVersion(APP_VERSION);
    a.setStyle("Fusion");

    try {
        QCommandLineParser parser;
        parser.setApplicationDescription(a.applicationName());
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("file", QCoreApplication::translate("main", "Circuit file to open."));

        QCommandLineOption arduinoFileOption(
                    {"a", "arduino-file"},
                    QCoreApplication::translate("main", "Export circuit to <arduino-file>"),
                    QCoreApplication::translate("main", "arduino file"));
        parser.addOption(arduinoFileOption);

        QCommandLineOption waveformFileOption(
                    {"w", "waveform"},
                    QCoreApplication::translate("main", "Export circuit to <waveform> text file"),
                    QCoreApplication::translate("main", "waveform text file"));
        parser.addOption(waveformFileOption);

        parser.process(a);


        QStringList args = parser.positionalArguments();
        MainWindow *w = new MainWindow(nullptr, (!args.empty() ? QString(args[0]) : QString()));

        QString arduFile = parser.value(arduinoFileOption);
        if (!arduFile.isEmpty()) {
            if (!args.empty()) {
                w->loadPandaFile(args[0]);
                return !w->exportToArduino(arduFile);
            }
            return 0;
        }
        QString wfFile = parser.value(waveformFileOption);
        if (!wfFile.isEmpty()) {
            if (!args.empty()) {
                w->loadPandaFile(args[0]);
                return !w->exportToWaveFormFile(wfFile);
            }
            return 0;
        }
        w->show();
        if (!args.empty()) {
            w->loadPandaFile(args[0]);
        }
    } catch (std::exception &e) {
        QMessageBox::critical(nullptr, "Erro", e.what());
        exit(1);
    }

    return a.exec();
}

/*
 * TODO: Tests for all elements
 * TODO: Create arduino version of all elements
 * TODO: Select some elements, and input wires become input buttons, output wires become leds... Connections are then
 * transferred to the IC's ports.
 */
