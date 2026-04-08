// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/CodeGen/TestSystemVerilogCodeGenUnit.h"

#include <QTemporaryDir>

#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
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

