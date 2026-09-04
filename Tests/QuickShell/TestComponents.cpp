// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestComponents.h"

#include <QFile>

#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputGND.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"

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
    // Port 1 is J: fixed to default to 0 (Reset mode, J=0/K=1) so the flip-flop's D input is
    // unconditionally 0 and the circuit self-resolves from an Unknown power-on state, instead
    // of the old J=1/K=1 default (toggle mode, which could stay stuck at Unknown).
    QCOMPARE(ic->inputPort(1)->status(), Status::Inactive);
    QCOMPARE(ic->inputPort(2)->status(), Status::Error);
    QCOMPARE(ic->inputPort(3)->status(), Status::Active);
    QCOMPARE(ic->inputPort(4)->status(), Status::Active);
}

void TestComponents::testIC()
{
    // Use shared utility for examples directory path
    const QString examplesPath = QuickTestUtils::examplesDir();
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

    auto *connection1 = new Connection();
    connection1->setStartPort(clkButton->outputPort());
    connection1->setEndPort(ic->inputPort(2));

    auto *connection2 = new Connection();
    connection2->setStartPort(prstButton->outputPort());
    connection2->setEndPort(ic->inputPort(0));

    auto *connection3 = new Connection();
    connection3->setStartPort(ic->outputPort(0));
    connection3->setEndPort(led1->inputPort());

    auto *connection4 = new Connection();
    connection4->setStartPort(ic->outputPort(1));
    connection4->setEndPort(led2->inputPort());

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.addItem(led1);
    canvas.addItem(led2);
    canvas.addItem(clkButton);
    canvas.addItem(prstButton);
    canvas.addItem(ic);
    canvas.addItem(connection1);
    canvas.addItem(connection2);
    canvas.addItem(connection3);
    canvas.addItem(connection4);

    Simulation *simulation = canvas.simulation();
    simulation->initialize();

    // Repeat the identical clk/prst cycle 10 times (not just once) to catch a sequential-state
    // bug that only manifests after repeated toggling -- e.g. edge-detection state leaking
    // between cycles or an internal counter drifting -- rather than only proving the JK
    // flip-flop resolves correctly from its initial power-on state.
    for (int i = 0; i < 10; ++i) {
        clkButton->setOff();
        prstButton->setOff();
        simulation->update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        prstButton->setOn();
        simulation->update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        simulation->update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOn();
        simulation->update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Active);

        QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);
        QCOMPARE(ic->outputPort(1)->status(), Status::Active);
    }
}

void TestComponents::testICs()
{
    // Use shared utility for examples directory path
    const QDir examplesDir(QuickTestUtils::examplesDir());
    QVERIFY2(examplesDir.exists(),
             qPrintable(QString("Examples directory not found: %1").arg(examplesDir.absolutePath())));

    const auto files = examplesDir.entryInfoList(QStringList{"*.panda"});
    QVERIFY2(!files.isEmpty(),
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
    QCOMPARE(loadedCount, files.size());
}
