// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "testcommands.h"
#include "testelements.h"
#include "testfiles.h"
#include "testicons.h"
#include "testlogicelements.h"
#include "testsimulationcontroller.h"
#include "testwaveform.h"
#include "thememanager.h"

#include <QtTest>

int main(int argc, char **argv)
{
    Comment::setVerbosity(-1);

    QApplication app(argc, argv);
    app.setOrganizationName("GIBIS-UNIFESP");
    app.setApplicationName("WiRedPanda");
    app.setApplicationVersion(APP_VERSION);

    ThemeManager::globalManager = new ThemeManager();

    int status = 0;
    status |= QTest::qExec(new TestCommands, argc, argv);
    status |= QTest::qExec(new TestElements, argc, argv);
    status |= QTest::qExec(new TestFiles, argc, argv);
    status |= QTest::qExec(new TestIcons, argc, argv);
    status |= QTest::qExec(new TestLogicElements, argc, argv);
    status |= QTest::qExec(new TestSimulationController, argc, argv);
    status |= QTest::qExec(new TestWaveForm, argc, argv);

    qInfo() << (status ? "Some test failed!" : "All tests have passed!");

    return status;
}
