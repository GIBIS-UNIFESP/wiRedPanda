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

#include <QtTest>

int main(int argc, char **argv)
{
    registerTypes();

    // Initialize resources from static library
    Q_INIT_RESOURCE(basic);
    Q_INIT_RESOURCE(dolphin);
    Q_INIT_RESOURCE(input);
    Q_INIT_RESOURCE(memory_dark);
    Q_INIT_RESOURCE(memory_light);
    Q_INIT_RESOURCE(misc);
    Q_INIT_RESOURCE(output);
    Q_INIT_RESOURCE(toolbar);
    Q_INIT_RESOURCE(translations);

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

    qInfo() << (status != 0 ? "Some test failed!" : "All tests have passed!");

    return status;
}
