// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "common.h"
#include "registertypes.h"
#include "testcommands.h"
#include "testelements.h"
#include "testfiles.h"
#include "testicons.h"
#include "testlogicelements.h"
#include "testsimulation.h"
#include "testwaveform.h"
#include "testarduino.h"
#include "testapplication.h"
#include "testdialogs.h"
#include "testscene.h"
#include "testtrashbutton.h"
#include "testrecentfiles.h"

#include <QtTest>

int main(int argc, char **argv)
{
    registerTypes();

    Comment::setVerbosity(-1);

    Application app(argc, argv);
    app.setOrganizationName("GIBIS-UNIFESP");
    app.setApplicationName("wiRedPanda");
    app.setApplicationVersion(APP_VERSION);

    int status = 0;
    
    // Create test objects on the stack to ensure proper cleanup
    TestCommands testCommands;
    TestElements testElements;
    TestFiles testFiles;
    TestIcons testIcons;
    TestLogicElements testLogicElements;
    TestSimulation testSimulation;
    TestWaveForm testWaveForm;
    TestArduino testArduino;
    TestApplication testApplication;
    TestDialogs testDialogs;
    TestScene testScene;
    TestTrashButton testTrashButton;
    TestRecentFiles testRecentFiles;
    
    status |= QTest::qExec(&testCommands, argc, argv);
    status |= QTest::qExec(&testElements, argc, argv);
    status |= QTest::qExec(&testFiles, argc, argv);
    status |= QTest::qExec(&testIcons, argc, argv);
    status |= QTest::qExec(&testLogicElements, argc, argv);
    status |= QTest::qExec(&testSimulation, argc, argv);
    status |= QTest::qExec(&testWaveForm, argc, argv);
    status |= QTest::qExec(&testArduino, argc, argv);
    status |= QTest::qExec(&testApplication, argc, argv);
    status |= QTest::qExec(&testDialogs, argc, argv);
    status |= QTest::qExec(&testScene, argc, argv);
    status |= QTest::qExec(&testTrashButton, argc, argv);
    status |= QTest::qExec(&testRecentFiles, argc, argv);

    qInfo() << (status != 0 ? "Some test failed!" : "All tests have passed!");

    return status;
}
