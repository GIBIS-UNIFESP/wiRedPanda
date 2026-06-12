// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/CodeGen/TestSystemVerilogCodeGenUnit.h"

#include <QRegularExpression>
#include <QTemporaryDir>

#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

namespace {
QVector<GraphicElement *> sceneElements(Scene *scene)
{
    QVector<GraphicElement *> result;
    for (auto *item : scene->items()) {
        if (item->type() == GraphicElement::Type)
            result.append(static_cast<GraphicElement *>(item));
    }
    return result;
}
} // namespace

void TestSystemVerilogCodeGenUnit::testAndGateCircuit()
{
    // InputSwitch → AND → Led — exercises declareInputs, declareOutputs, assignVariablesRec, loop
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    auto *andGate = new And;
    auto *led = new Led;

    builder.add(sw1, sw2, andGate, led);
    builder.connect(sw1, 0, andGate, 0);
    builder.connect(sw2, 0, andGate, 1);
    builder.connect(andGate, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/and_test.sv";

    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("module"));
    QVERIFY(content.contains("input"));
    QVERIFY(content.contains("output"));
    QVERIFY(content.contains("assign"));

    QVERIFY(!codegen.inputMap().isEmpty());
    QVERIFY(!codegen.outputMap().isEmpty());
}

void TestSystemVerilogCodeGenUnit::testOrNotCircuit()
{
    // InputSwitch → OR → NOT → Led
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    auto *orGate = new Or;
    auto *notGate = new Not;
    auto *led = new Led;

    builder.add(sw1, sw2, orGate, notGate, led);
    builder.connect(sw1, 0, orGate, 0);
    builder.connect(sw2, 0, orGate, 1);
    builder.connect(orGate, 0, notGate, 0);
    builder.connect(notGate, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/or_not_test.sv";

    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("module"));
    QVERIFY(content.contains("assign"));
    QCOMPARE(codegen.inputMap().size(), 2);
    QCOMPARE(codegen.outputMap().size(), 1);
}

void TestSystemVerilogCodeGenUnit::testMultiGateChain()
{
    // sw1 → AND ─┐
    //             ├→ OR → Led
    // sw2 ───────┘
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    auto *sw3 = new InputSwitch;
    auto *andGate = new And;
    auto *orGate = new Or;
    auto *led = new Led;

    builder.add(sw1, sw2, sw3, andGate, orGate, led);
    builder.connect(sw1, 0, andGate, 0);
    builder.connect(sw2, 0, andGate, 1);
    builder.connect(andGate, 0, orGate, 0);
    builder.connect(sw3, 0, orGate, 1);
    builder.connect(orGate, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/chain_test.sv";

    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("module"));
    QCOMPARE(codegen.inputMap().size(), 3);
    QCOMPARE(codegen.outputMap().size(), 1);
}

void TestSystemVerilogCodeGenUnit::testMuxCircuit()
{
    // Mux with switches connected — exercises Mux case generation in assignVariablesRec
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    auto *swSel = new InputSwitch;
    auto *mux = new Mux;
    auto *led = new Led;

    builder.add(sw1, sw2, swSel, mux, led);
    builder.connect(sw1, 0, mux, 0);   // data 0
    builder.connect(sw2, 0, mux, 1);   // data 1
    builder.connect(swSel, 0, mux, 2); // selector
    builder.connect(mux, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/mux_test.sv";

    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("module"));
    QVERIFY(content.contains("case") || content.contains("assign"));
}

void TestSystemVerilogCodeGenUnit::testDemuxCircuit()
{
    // Demux with switch connected
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *swData = new InputSwitch;
    auto *swSel = new InputSwitch;
    auto *demux = new Demux;
    auto *led1 = new Led;
    auto *led2 = new Led;

    builder.add(swData, swSel, demux, led1, led2);
    builder.connect(swData, 0, demux, 0); // data
    builder.connect(swSel, 0, demux, 1);  // selector
    builder.connect(demux, 0, led1, 0);
    builder.connect(demux, 1, led2, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/demux_test.sv";

    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("module"));
    QCOMPARE(codegen.outputMap().size(), 2);
}

void TestSystemVerilogCodeGenUnit::testEmptyScene()
{
    WorkSpace workspace;

    QTemporaryDir dir;
    QString path = dir.path() + "/empty_test.sv";

    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("module"));
    QVERIFY(codegen.inputMap().isEmpty());
    QVERIFY(codegen.outputMap().isEmpty());
}

void TestSystemVerilogCodeGenUnit::testUnwritablePathThrows()
{
    // Regression test (F22): the constructor used to return silently when the
    // file could not be opened; generate() then streamed into a device-less
    // QTextStream and the UI reported success with no file written.
    bool threw = false;
    try {
        SystemVerilogCodeGen codegen("/nonexistent-dir/does/not/exist.sv", {});
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY(threw);
}

void TestSystemVerilogCodeGenUnit::testTruthTableMultiOutput()
{
    // Regression test (F19): only output 0 of a multi-output TruthTable was
    // emitted — outputs 1..N silently held their initial value in generated
    // code. Output k must read key bits 256*k + row.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    auto *tt = new TruthTable;
    tt->setOutputSize(2);
    auto *led0 = new Led;
    auto *led1 = new Led;

    builder.add(sw1, sw2, tt, led0, led1);
    builder.connect(sw1, 0, tt, 0);
    builder.connect(sw2, 0, tt, 1);
    builder.connect(tt, 0, led0, 0);
    builder.connect(tt, 1, led1, 0);

    // Output 0 = AND (row 3 only), output 1 = OR (rows 1..3).
    tt->key().setBit(3, true);
    tt->key().setBit(256 + 1, true);
    tt->key().setBit(256 + 2, true);
    tt->key().setBit(256 + 3, true);

    QTemporaryDir dir;
    QString path = dir.path() + "/tt_multi.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QString content = file.readAll();

    // Two emitted blocks, with the per-output truth values: row 01 is true
    // only for the OR output; row 11 is true for both.
    QCOMPARE(content.count("//TruthTable\n"), 2); // one begin marker per output
    QCOMPARE(content.count(QRegularExpression("2'b01: \\S+ = 1'b1;")), 1);
    QCOMPARE(content.count(QRegularExpression("2'b11: \\S+ = 1'b1;")), 2);
    QCOMPARE(content.count(QRegularExpression("2'b00: \\S+ = 1'b0;")), 2);
}

void TestSystemVerilogCodeGenUnit::testRotaryInputsDeclared()
{
    // Regression test (F23): rotary outputs received aux wire declarations
    // with no driver — the module floated (z) wherever a rotary fed logic.
    // Each rotary position is a one-hot module input now.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *rotary = new InputRotary;
    rotary->setOutputSize(4);
    rotary->setLabel("rot");
    QVector<Led *> leds(4);
    builder.add(rotary);
    for (int i = 0; i < 4; ++i) {
        leds[i] = new Led;
        builder.add(leds[i]);
        builder.connect(rotary, i, leds[i], 0);
    }

    QTemporaryDir dir;
    QString path = dir.path() + "/rotary.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QString content = file.readAll();

    for (int i = 0; i < 4; ++i) {
        QVERIFY2(content.contains(QRegularExpression(QString("input \\S*_rot_%1").arg(i))),
                 qPrintable(QString("missing module input for rotary position %1").arg(i)));
    }
    QVERIFY2(!content.contains("aux_"), "rotary must not leave undriven aux wires");
}
