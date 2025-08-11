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
#include "testelementfactory.h"
#include "testserialization.h"
#include "testgraphicelement.h"
#include "testenums.h"
#include "testelementlabel.h"
#include "testic.h"
#include "testsettings.h"
#include "testthememanager.h"
#include "testworkspace.h"
#include "testcommon.h"
#include "testsimulationblocker.h"
#include "testsimulationtester.h"
#include "testelementmapping.h"
#include "testqneconnection_safe.h"
#include "testqneconnection_advanced.h"
#include "testqneport_advanced.h"

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
    TestElementFactory testElementFactory;
    TestSerialization testSerialization;
    TestGraphicElement testGraphicElement;
    TestEnums testEnums;
    TestElementLabel testElementLabel;
    TestIC testIC;
    TestSettings testSettings;
    TestThemeManager testThemeManager;
    TestWorkspace testWorkspace;
    TestCommon testCommon;
    TestSimulationBlocker testSimulationBlocker;
    TestSimulationTester testSimulationTester;
    TestElementMapping testElementMapping;
    TestQNEConnectionSafe testQNEConnectionSafe;
    TestQNEConnectionAdvanced testQNEConnectionAdvanced;
    TestQNEPortAdvanced testQNEPortAdvanced;
    
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
    status |= QTest::qExec(&testElementFactory, argc, argv);
    status |= QTest::qExec(&testSerialization, argc, argv);
    status |= QTest::qExec(&testGraphicElement, argc, argv);
    status |= QTest::qExec(&testEnums, argc, argv);
    status |= QTest::qExec(&testElementLabel, argc, argv);
    status |= QTest::qExec(&testIC, argc, argv);
    status |= QTest::qExec(&testSettings, argc, argv);
    status |= QTest::qExec(&testThemeManager, argc, argv);
    status |= QTest::qExec(&testWorkspace, argc, argv);
    status |= QTest::qExec(&testCommon, argc, argv);
    status |= QTest::qExec(&testSimulationBlocker, argc, argv);
    status |= QTest::qExec(&testSimulationTester, argc, argv);
    status |= QTest::qExec(&testElementMapping, argc, argv);
    status |= QTest::qExec(&testQNEConnectionSafe, argc, argv);
    status |= QTest::qExec(&testQNEConnectionAdvanced, argc, argv);
    status |= QTest::qExec(&testQNEPortAdvanced, argc, argv);

    qInfo() << (status != 0 ? "Some test failed!" : "All tests have passed!");

    return status;
}
