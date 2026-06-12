// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/System/TestWaveform.h"

#include <QTemporaryFile>

// Standard headers
#include <exception>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEPort.h"

#include "Tests/Common/TestUtils.h"

void TestWaveform::testDisplay4Bits()
{
    WorkSpace workspace;

    const QDir examplesDir(TestUtils::examplesDir());
    const QString fileName = examplesDir.absoluteFilePath("display-4bits.panda");

    QFile pandaFile(fileName);
    QVERIFY2(pandaFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(pandaFile.fileName(), pandaFile.errorString())));
    QDataStream stream(&pandaFile);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
        workspace.load(stream, version, examplesDir.absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load %1: %2").arg(fileName, e.what())));
        return;
    }

    BewavedDolphin bewavedDolphin(workspace.scene(), false);
    try {
        bewavedDolphin.createWaveform("");
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to create waveform from %1: %2").arg(fileName, e.what())));
        return;
    }

    QTemporaryFile tempFile;
    QVERIFY2(tempFile.open(),
             qPrintable(QString("Failed to open temporary file: %1")
                       .arg(tempFile.errorString())));
    QTextStream tempStream(&tempFile);
    bewavedDolphin.saveToTxt(tempStream);
    tempStream.flush();
    QVERIFY2(tempFile.reset(), "Failed to reset temporary file position");

    QFile referenceFile(examplesDir.absoluteFilePath("display-4bits.txt"));

    QVERIFY2(referenceFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(referenceFile.fileName(), referenceFile.errorString())));

    QCOMPARE(tempFile.readAll().replace("\r\n", "\n"), referenceFile.readAll().replace("\r\n", "\n"));
}

void TestWaveform::testRestoreInputsAfterSweep()
{
    // Regression test (F7): loadSignals() snapshots input values per *port*
    // while restoreInputs() consumed them per *element*. With a multi-port
    // input (rotary) placed before other inputs, the wrong saved values were
    // applied to every subsequent element, and the rotary itself was
    // "restored" by a setOn(value, port) loop that just left the last port
    // selected. Closing the waveform sweep silently corrupted circuit state.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *rotary = new InputRotary();
    rotary->setOutputSize(4);
    auto *swOn = new InputSwitch();
    auto *swOff = new InputSwitch();
    auto *led0 = new Led();
    auto *led1 = new Led();
    auto *led2 = new Led();

    builder.add(rotary);
    builder.add(swOn, swOff, led0, led1, led2);
    builder.connect(rotary, 2, led0, 0);
    builder.connect(swOn, 0, led1, 0);
    builder.connect(swOff, 0, led2, 0);

    rotary->setOn(true, 2);
    swOn->setOn(true);
    swOff->setOn(false);

    BewavedDolphin bewavedDolphin(workspace.scene(), false);
    bewavedDolphin.createWaveform("");

    // The sweep must hand the circuit back exactly as it found it.
    QCOMPARE(rotary->outputPort(0)->status(), Status::Inactive);
    QCOMPARE(rotary->outputPort(1)->status(), Status::Inactive);
    QCOMPARE(rotary->outputPort(2)->status(), Status::Active);
    QCOMPARE(rotary->outputPort(3)->status(), Status::Inactive);
    QCOMPARE(swOn->outputPort()->status(), Status::Active);
    QCOMPARE(swOff->outputPort()->status(), Status::Inactive);
}

void TestWaveform::testSignalRowEnumerationMatchesModel()
{
    // Companion to F20: the waveform model holds one row per *port*. The MCP
    // create_waveform handler used to index rows by element, misaligning
    // every row after a multi-port input/output. It now consumes the
    // dolphin's canonical row enumeration — this test pins that enumeration
    // to the model layout.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *rotary = new InputRotary();
    rotary->setOutputSize(4);
    rotary->setLabel("Rot");
    auto *sw = new InputSwitch();
    sw->setLabel("Sw");
    auto *display = new Display7();
    display->setLabel("Disp");
    auto *led = new Led();
    led->setLabel("L");

    builder.add(rotary, sw, display, led);
    builder.connect(rotary, 0, display, 0);
    builder.connect(sw, 0, led, 0);

    BewavedDolphin bewavedDolphin(workspace.scene(), false);
    bewavedDolphin.createWaveform("");

    const QStringList inputRows = bewavedDolphin.inputRowLabels();
    const QStringList outputRows = bewavedDolphin.outputRowLabels();

    // 4 rotary ports + 1 switch = 5 input rows; 8 display ports + 1 led = 9
    // output rows; the model stacks inputs first, outputs after.
    QCOMPARE(inputRows.size(), 5);
    QCOMPARE(outputRows.size(), 9);
    QCOMPARE(bewavedDolphin.getModel()->rowCount(), inputRows.size() + outputRows.size());

    QVERIFY(inputRows.contains("Sw"));
    for (int port = 0; port < 4; ++port) {
        QVERIFY(inputRows.contains(QString("Rot[%1]").arg(port)));
    }
    // Ports of one element occupy contiguous rows in declaration order.
    QCOMPARE(inputRows.indexOf("Rot[3]") - inputRows.indexOf("Rot[0]"), 3);

    QVERIFY(outputRows.contains("L"));
    for (int port = 0; port < 8; ++port) {
        QVERIFY(outputRows.contains(QString("Disp[%1]").arg(port)));
    }
}
