// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include <iostream>

#include "common.h"
#include "testcommands.h"
#include "testelements.h"
#include "testfiles.h"
#include "testicons.h"
#include "testlogicelements.h"
#include "testsimulationcontroller.h"
#include "testwaveform.h"
#include "thememanager.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    ThemeManager::globalMngr = new ThemeManager();
    Comment::setVerbosity(-1);
    a.setOrganizationName("WPanda");
    a.setApplicationName("WiredPanda");
    a.setApplicationVersion(APP_VERSION);
    TestElements testElements;
    TestLogicElements testLogicElements;
    TestSimulationController testSC;
    TestFiles testFiles;
    TestCommands testCommands;
    TestWaveForm testWf;
    TestIcons testIcons;
    int status = 0;
    status |= QTest::qExec(&testElements, argc, argv);
    status |= QTest::qExec(&testLogicElements, argc, argv);
    status |= QTest::qExec(&testSC, argc, argv);
    status |= QTest::qExec(&testFiles, argc, argv);
    status |= QTest::qExec(&testCommands, argc, argv);
    status |= QTest::qExec(&testWf, argc, argv);
    status |= QTest::qExec(&testIcons, argc, argv);

    std::cout << (status ? "Some test failed!" : "All tests have passed!") << std::endl;

    return status;
}