// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "registertypes.h"
#include "testarduino.h"
#include "testcircuitintegration.h"
#include "testcommands.h"
#include "testelementeditor.h"
#include "testfiles.h"
#include "testgraphicelements.h"
#include "testicons.h"
#include "testlogiccore.h"
#include "testmainwindow.h"
#include "testserializationregression.h"
#include "testsimulationworkflow.h"
#include "testuiinteraction.h"
#include "testwaveform.h"

#include <QtTest>
#include <QLoggingCategory>
#include <QTextStream>


// Custom message handler to suppress specific Qt framework warnings during testing
void testMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    // Suppress specific Qt framework warnings that are not relevant for tests
    if (type == QtWarningMsg) {
        // Suppress propagateSizeHints warning from offscreen platform plugin
        if (msg.contains("This plugin does not support propagateSizeHints()")) {
            return;
        }
        // Suppress SVG path data warnings during icon loading
        if (msg.contains("Invalid path data; path truncated.")) {
            return;
        }
    }
    
    // Let all other messages through using Qt's default formatting
    QTextStream stream(stderr);
    const char* typeStr = "";
    switch (type) {
    case QtDebugMsg:    typeStr = "Debug"; break;
    case QtWarningMsg:  typeStr = "Warning"; break;
    case QtCriticalMsg: typeStr = "Critical"; break;
    case QtFatalMsg:    typeStr = "Fatal"; break;
    case QtInfoMsg:     typeStr = "Info"; break;
    }
    stream << typeStr << ": " << msg << Qt::endl;
}

int main(int argc, char **argv)
{
    registerTypes();

    Comment::setVerbosity(-1);
    
    // Install custom message handler to suppress Qt framework warnings
    qInstallMessageHandler(testMessageHandler);

    QApplication app(argc, argv);
    app.setOrganizationName("GIBIS-UNIFESP");
    app.setApplicationName("wiRedPanda");
    app.setApplicationVersion(APP_VERSION);

    int status = 0;
    // Core logic tests (existing, excellent coverage)
    status |= QTest::qExec(new TestLogicCore(), argc, argv);
    status |= QTest::qExec(new TestGraphicElements(), argc, argv);
    status |= QTest::qExec(new TestCircuitIntegration(), argc, argv);
    status |= QTest::qExec(new TestSimulationWorkflow(), argc, argv);
    status |= QTest::qExec(new TestCommands(), argc, argv);
    status |= QTest::qExec(new TestFiles(), argc, argv);
    status |= QTest::qExec(new TestIcons(), argc, argv);
    status |= QTest::qExec(new TestSerializationRegression(), argc, argv);
    status |= QTest::qExec(new TestUIInteraction(), argc, argv);
    status |= QTest::qExec(new TestWaveForm(), argc, argv);

    // New comprehensive tests for previously uncovered areas
    status |= QTest::qExec(new TestArduino(), argc, argv);           // Arduino code generation (was 0% coverage)
    status |= QTest::qExec(new TestElementEditor(), argc, argv);     // Element editor UI (was 0% coverage)
    status |= QTest::qExec(new TestMainWindow(), argc, argv);        // Main application window (was 0% coverage)

    qInfo() << (status != 0 ? "Some test failed!" : "All tests have passed!");

    return status;
}
