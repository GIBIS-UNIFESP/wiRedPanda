// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestComponents.h"

#include <QFile>

#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputGND.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Common/TestUtils.h"

void TestComponents::testNode()
{
    Node elm;
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::Node);
}

void TestComponents::testVCC()
{
    InputVcc vcc;
    QCOMPARE(vcc.outputSize(), 1);
    QCOMPARE(vcc.inputSize(), 0);
    QCOMPARE(vcc.outputPort()->status(), Status::Active);
}

void TestComponents::testGND()
{
    InputGnd gnd;
    QCOMPARE(gnd.outputSize(), 1);
    QCOMPARE(gnd.inputSize(), 0);
    QCOMPARE(gnd.outputPort()->status(), Status::Inactive);
}

void TestComponents::testICData(IC *ic)
{
    QCOMPARE(ic->inputSize(), 5);
    QCOMPARE(ic->outputSize(), 2);

    QCOMPARE(ic->inputPort(0)->isRequired(), false);
    QCOMPARE(ic->inputPort(1)->isRequired(), false);
    QCOMPARE(ic->inputPort(2)->isRequired(), true);
    QCOMPARE(ic->inputPort(3)->isRequired(), false);
    QCOMPARE(ic->inputPort(4)->isRequired(), false);

    QCOMPARE(ic->inputPort(0)->status(), Status::Active);
    QCOMPARE(ic->inputPort(1)->status(), Status::Active);
    QCOMPARE(ic->inputPort(2)->status(), Status::Unknown);
    QCOMPARE(ic->inputPort(3)->status(), Status::Active);
    QCOMPARE(ic->inputPort(4)->status(), Status::Active);
}

void TestComponents::testIC()
{
    // Use shared utility for examples directory path
    const QString examplesPath = TestUtils::examplesDir();
    const QString icFile = examplesPath + "jkflipflop.panda";

    // Verify file exists before attempting to load
    QVERIFY2(QFile::exists(icFile),
             qPrintable(QString("Test IC file not found: %1").arg(icFile)));

    auto *ic = new IC();

    // Load file with error handling
    try {
        ic->loadFile(icFile, examplesPath);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    testICData(ic);

    auto *clkButton = new InputButton();
    auto *prstButton = new InputButton();

    auto *led1 = new Led();
    auto *led2 = new Led();

    auto *connection1 = new QNEConnection();
    connection1->setStartPort(clkButton->outputPort());
    connection1->setEndPort(ic->inputPort(2));

    auto *connection2 = new QNEConnection();
    connection2->setStartPort(prstButton->outputPort());
    connection2->setEndPort(ic->inputPort(0));

    auto *connection3 = new QNEConnection();
    connection3->setStartPort(ic->outputPort(0));
    connection3->setEndPort(led1->inputPort());

    auto *connection4 = new QNEConnection();
    connection4->setStartPort(ic->outputPort(1));
    connection4->setEndPort(led2->inputPort());

    Scene scene;
    scene.addItem(led1);
    scene.addItem(led2);
    scene.addItem(clkButton);
    scene.addItem(prstButton);
    scene.addItem(ic);
    scene.addItem(connection1);
    scene.addItem(connection2);
    scene.addItem(connection3);
    scene.addItem(connection4);

    Simulation simulation(&scene);
    simulation.initialize();

    for (int i = 0; i < 10; ++i) {
        clkButton->setOff();
        prstButton->setOff();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        prstButton->setOn();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOn();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Active);

        QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);
        QCOMPARE(ic->outputPort(1)->status(), Status::Active);
    }
}

void TestComponents::testICs()
{
    // Use shared utility for examples directory path
    const QDir examplesDir(TestUtils::examplesDir());
    QVERIFY2(examplesDir.exists(),
             qPrintable(QString("Examples directory not found: %1").arg(examplesDir.absolutePath())));

    const auto files = examplesDir.entryInfoList(QStringList{"*.panda"});
    QVERIFY2(files.count() > 0,
             qPrintable(QString("No .panda example files found in: %1").arg(examplesDir.absolutePath())));

    int loadedCount = 0;
    for (const auto &fileInfo : files) {
        IC ic;

        // Load file with error handling
        try {
            ic.loadFile(fileInfo.absoluteFilePath(), fileInfo.absolutePath());
            loadedCount++;
        } catch (const std::exception &e) {
            QFAIL(qPrintable(QString("Failed to load IC file %1: %2")
                              .arg(fileInfo.fileName()).arg(e.what())));
        }
    }

    // Verify at least one file was successfully loaded
    QCOMPARE(loadedCount, files.count());
}

