// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/CodeGen/TestArduinoCodeGenUnit.h"

#include <QRegularExpression>
#include <QTemporaryDir>

#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
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
QVector<GraphicElement *> arduinoSceneElements(Scene *scene)
{
    QVector<GraphicElement *> result;
    for (auto *item : scene->items()) {
        if (item->type() == GraphicElement::Type)
            result.append(static_cast<GraphicElement *>(item));
    }
    return result;
}
} // namespace

void TestArduinoCodeGenUnit::testAndGateCircuit()
{
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
    QString path = dir.path() + "/and_test.ino";

    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("void setup()"));
    QVERIFY(content.contains("void loop()"));
    QVERIFY(content.contains("pinMode"));
}

void TestArduinoCodeGenUnit::testMultiGateCircuit()
{
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
    QString path = dir.path() + "/multi_test.ino";

    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("void setup()"));
    QVERIFY(content.contains("digitalRead"));
}

void TestArduinoCodeGenUnit::testMuxCircuit()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    auto *swSel = new InputSwitch;
    auto *mux = new Mux;
    auto *led = new Led;

    builder.add(sw1, sw2, swSel, mux, led);
    builder.connect(sw1, 0, mux, 0);
    builder.connect(sw2, 0, mux, 1);
    builder.connect(swSel, 0, mux, 2);
    builder.connect(mux, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/mux_test.ino";

    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("void setup()"));
}

void TestArduinoCodeGenUnit::testDemuxCircuit()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *swData = new InputSwitch;
    auto *swSel = new InputSwitch;
    auto *demux = new Demux;
    auto *led1 = new Led;
    auto *led2 = new Led;

    builder.add(swData, swSel, demux, led1, led2);
    builder.connect(swData, 0, demux, 0);
    builder.connect(swSel, 0, demux, 1);
    builder.connect(demux, 0, led1, 0);
    builder.connect(demux, 1, led2, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/demux_test.ino";

    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("void setup()"));
}

void TestArduinoCodeGenUnit::testEmptyScene()
{
    WorkSpace workspace;

    QTemporaryDir dir;
    QString path = dir.path() + "/empty_test.ino";

    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = file.readAll();

    QVERIFY(content.contains("void setup()"));
}

void TestArduinoCodeGenUnit::testTruthTableMultiOutput()
{
    // Regression test (F19): only output 0 of a multi-output TruthTable was
    // emitted — outputs 1..N silently held their initial value in the sketch.
    // Output k must read key bits 256*k + row.
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
    QString path = dir.path() + "/tt_multi.ino";
    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QString content = file.readAll();

    // Two emitted chains; rows 1 and 2 are HIGH only for the OR output,
    // row 3 for both.
    QCOMPARE(content.count("//TruthTable\n"), 2); // one begin marker per output
    QCOMPARE(content.count(QRegularExpression("== 1\\) \\{\\n        \\S+ = HIGH;")), 1);
    QCOMPARE(content.count(QRegularExpression("== 3\\) \\{\\n        \\S+ = HIGH;")), 2);
}
