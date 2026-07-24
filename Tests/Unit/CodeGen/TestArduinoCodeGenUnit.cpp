// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/CodeGen/TestArduinoCodeGenUnit.h"

#include <QRegularExpression>
#include <QTemporaryDir>

#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    // Two emitted chains; rows 1 and 2 are HIGH only for the OR output,
    // row 3 for both.
    QCOMPARE(content.count("//TruthTable\n"), 2); // one begin marker per output
    QCOMPARE(content.count(QRegularExpression("== 1\\) \\{\\n        \\S+ = HIGH;")), 1);
    QCOMPARE(content.count(QRegularExpression("== 3\\) \\{\\n        \\S+ = HIGH;")), 2);
}

void TestArduinoCodeGenUnit::testSRLatchCircuit()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *swS = new InputSwitch;
    auto *swR = new InputSwitch;
    auto *latch = new SRLatch;
    auto *ledQ = new Led;
    auto *ledNotQ = new Led;

    builder.add(swS, swR, latch, ledQ, ledNotQ);
    builder.connect(swS, 0, latch, 0);
    builder.connect(swR, 0, latch, 1);
    builder.connect(latch, 0, ledQ, 0);
    builder.connect(latch, 1, ledNotQ, 0);

    QTemporaryDir dir;
    const QString path = dir.path() + "/srlatch_test.ino";
    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY(content.contains("//SR Latch"));
    QVERIFY(content.contains("//End of SR Latch"));
    QVERIFY(content.contains("if (g_sample) {"));
}

void TestArduinoCodeGenUnit::testGenerateTestbenchDeclaresSequentialState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *clk = new Clock;
    auto *sw = new InputSwitch;
    auto *dff = new DFlipFlop;
    auto *tff = new TFlipFlop;
    auto *jkff = new JKFlipFlop;
    auto *srff = new SRFlipFlop;
    auto *led1 = new Led;
    auto *led2 = new Led;
    auto *led3 = new Led;
    auto *led4 = new Led;

    builder.add(clk, sw, dff, tff, jkff, srff, led1, led2, led3, led4);
    builder.connect(sw, 0, dff, 0);
    builder.connect(clk, 0, dff, 1);
    builder.connect(dff, 0, led1, 0);
    builder.connect(sw, 0, tff, 0);
    builder.connect(clk, 0, tff, 1);
    builder.connect(tff, 0, led2, 0);
    builder.connect(sw, 0, jkff, 0);
    builder.connect(clk, 0, jkff, 1);
    builder.connect(sw, 0, jkff, 2);
    builder.connect(jkff, 0, led3, 0);
    builder.connect(sw, 0, srff, 0);
    builder.connect(clk, 0, srff, 1);
    builder.connect(sw, 0, srff, 2);
    builder.connect(srff, 0, led4, 0);

    QTemporaryDir dir;
    const QString sketchPath = dir.path() + "/seq.ino";
    ArduinoCodeGen codegen(sketchPath, arduinoSceneElements(workspace.scene()));
    codegen.generate(); // populates m_varMap/m_declaredVariables/m_hasSequential

    const QString tbPath = dir.path() + "/seq_tb.ino";
    codegen.generateTestbench(tbPath, {});

    QFile tbFile(tbPath);
    QVERIFY(tbFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = tbFile.readAll();

    // declareSequentialStateRec's per-type state declarations, only reachable via
    // generateTestbench() (not the main sketch).
    QVERIFY2(content.contains("elapsedMillis") && content.contains("_interval = 1000;"),
             "Top-level Clock state must be declared in the testbench");
    QCOMPARE(content.count("_inclk = LOW;"), 4); // D, T, JK, SR flip-flops each declare one
    QCOMPARE(content.count("_last = LOW;"), 1);  // only DFlipFlop declares this
    QVERIFY2(content.contains("g_sample = true;"), "Sequential testbench must use the non-blocking tick driver");
}

void TestArduinoCodeGenUnit::testTooManyInputRotaryPositionsThrowsPinOverflow()
{
    // Every input pin is claimed by InputRotary elements only (36 of them, at least 2 ports
    // each — comfortably past the largest board's 70 pins). Since every element competing
    // for pins is the same type, whichever one is mid-loop when the pins run out throws from
    // the exact same source line regardless of scene iteration order — unlike mixing
    // switches and a rotary, where order determines which element's overflow check fires.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    QVector<InputRotary *> rotaries;
    for (int i = 0; i < 36; ++i) {
        auto *rotary = new InputRotary;
        QVERIFY2(rotary->outputSize() >= 2, "InputRotary should have at least 2 output ports by default");
        rotaries.append(rotary);
        builder.add(rotary);
    }

    QTemporaryDir dir;
    const QString path = dir.path() + "/overflow_test.ino";
    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));

    QVERIFY_THROWS(std::exception, codegen.generate());
}

void TestArduinoCodeGenUnit::testGenerateTestbenchUnwritablePathThrows()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir lockedDir;
    QVERIFY(lockedDir.isValid());
    const QString tbPath = lockedDir.path() + "/tb.ino";
    QVERIFY(QFile::setPermissions(lockedDir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    // Sanity: confirm a plain QFile really can't be created under this directory on this
    // system before relying on it below.
    {
        QFile probe(tbPath);
        QVERIFY(!probe.open(QIODevice::WriteOnly));
    }

    QTemporaryDir writableDir;
    QVERIFY(writableDir.isValid());
    ArduinoCodeGen codegen(writableDir.path() + "/main.ino", {});

    QVERIFY_THROWS(std::exception, codegen.generateTestbench(tbPath, {}));

    QFile::setPermissions(lockedDir.path(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
#endif
}

void TestArduinoCodeGenUnit::testTruthTableInputTiedToVccResolvesToHigh()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *vcc = new InputVcc;
    auto *sw = new InputSwitch;
    auto *tt = new TruthTable;
    auto *led = new Led;

    builder.add(vcc, sw, tt, led);
    builder.connect(vcc, 0, tt, 0); // always-HIGH input
    builder.connect(sw, 0, tt, 1);
    builder.connect(tt, 0, led, 0);
    tt->key().setBit(2, true);
    tt->key().setBit(3, true);

    QTemporaryDir dir;
    const QString path = dir.path() + "/tt_vcc_input.ino";
    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    // The VCC-driven input (index 0) must resolve to the literal "1" in the index
    // expression, not a variable name.
    QVERIFY2(content.contains("(1 << 1)"), qPrintable(content));
}

void TestArduinoCodeGenUnit::testGenerateTestbenchNonSequentialSettleLoop()
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
    const QString sketchPath = dir.path() + "/nonseq.ino";
    ArduinoCodeGen codegen(sketchPath, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    const QString tbPath = dir.path() + "/nonseq_tb.ino";
    codegen.generateTestbench(tbPath, {});

    QFile tbFile(tbPath);
    QVERIFY(tbFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = tbFile.readAll();

    // Purely combinational: no g_sample/commitFlipFlops, just a plain settle loop.
    QVERIFY(!content.contains("g_sample"));
    QVERIFY2(content.contains("for (int s = 0; s <"), qPrintable(content));
}

void TestArduinoCodeGenUnit::testDemuxWithFourOutputsUsesTwoSelectLines()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *swData = new InputSwitch;
    auto *swSel0 = new InputSwitch;
    auto *swSel1 = new InputSwitch;
    auto *demux = new Demux;
    demux->setOutputSize(4);
    auto *led0 = new Led;
    auto *led1 = new Led;
    auto *led2 = new Led;
    auto *led3 = new Led;

    builder.add(swData, swSel0, swSel1, demux, led0, led1, led2, led3);
    builder.connect(swData, 0, demux, 0);
    builder.connect(swSel0, 0, demux, 1);
    builder.connect(swSel1, 0, demux, 2);
    builder.connect(demux, 0, led0, 0);
    builder.connect(demux, 1, led1, 0);
    builder.connect(demux, 2, led2, 0);
    builder.connect(demux, 3, led3, 0);

    QTemporaryDir dir;
    const QString path = dir.path() + "/demux4_test.ino";
    ArduinoCodeGen codegen(path, arduinoSceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    // Two select lines means the select expression combines two bits via " << 1)".
    QVERIFY2(content.contains(" << 1)"), qPrintable(content));
    QVERIFY(content.contains("} else if (("));
}
