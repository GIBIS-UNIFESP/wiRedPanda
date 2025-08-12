// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include "testelementeditor.h"
#include "testgraphicsview.h"
#include "testdragdrop.h"
#include "testlengthdialog.h"
#include "testclockdialog.h"
#include "testproperties.h"
#include "testsceneadvanced.h"

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
    status |= QTest::qExec(new TestCommands(), argc, argv);
    status |= QTest::qExec(new TestElements(), argc, argv);
    status |= QTest::qExec(new TestFiles(), argc, argv);
    status |= QTest::qExec(new TestIcons(), argc, argv);
    status |= QTest::qExec(new TestLogicElements(), argc, argv);
    status |= QTest::qExec(new TestSimulation(), argc, argv);
    status |= QTest::qExec(new TestWaveForm(), argc, argv);
    status |= QTest::qExec(new TestArduino(), argc, argv);
    status |= QTest::qExec(new TestApplication(), argc, argv);
    status |= QTest::qExec(new TestDialogs(), argc, argv);
    status |= QTest::qExec(new TestScene(), argc, argv);
    status |= QTest::qExec(new TestTrashButton(), argc, argv);
    status |= QTest::qExec(new TestRecentFiles(), argc, argv);
    status |= QTest::qExec(new TestElementFactory(), argc, argv);
    status |= QTest::qExec(new TestSerialization(), argc, argv);
    status |= QTest::qExec(new TestGraphicElement(), argc, argv);
    status |= QTest::qExec(new TestEnums(), argc, argv);
    status |= QTest::qExec(new TestElementLabel(), argc, argv);
    status |= QTest::qExec(new TestIC(), argc, argv);
    status |= QTest::qExec(new TestSettings(), argc, argv);
    status |= QTest::qExec(new TestThemeManager(), argc, argv);
    status |= QTest::qExec(new TestWorkspace(), argc, argv);
    status |= QTest::qExec(new TestCommon(), argc, argv);
    status |= QTest::qExec(new TestSimulationBlocker(), argc, argv);
    status |= QTest::qExec(new TestSimulationTester(), argc, argv);
    status |= QTest::qExec(new TestElementMapping(), argc, argv);
    status |= QTest::qExec(new TestQNEConnectionSafe(), argc, argv);
    status |= QTest::qExec(new TestQNEConnectionAdvanced(), argc, argv);
    status |= QTest::qExec(new TestQNEPortAdvanced(), argc, argv);
    status |= QTest::qExec(new TestElementEditor(), argc, argv);
    status |= QTest::qExec(new TestGraphicsView(), argc, argv);
    status |= QTest::qExec(new TestDragDrop(), argc, argv);
    status |= QTest::qExec(new TestLengthDialog(), argc, argv);
    status |= QTest::qExec(new TestClockDialog(), argc, argv);
    status |= QTest::qExec(new TestProperties(), argc, argv);
    status |= QTest::qExec(new TestSceneAdvanced(), argc, argv);

    qInfo() << (status != 0 ? "Some test failed!" : "All tests have passed!");

    return status;
}
