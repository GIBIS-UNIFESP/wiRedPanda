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

int main(int argc, char **argv)
{
    registerTypes();

    Comment::setVerbosity(-1);

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
