// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/CodeGen/TestSystemVerilogCodeGenUnit.h"

#include <memory>

#include <QRegularExpression>
#include <QTemporaryDir>

#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();

    QVERIFY(content.contains("module"));
    QVERIFY(content.contains("input"));
    QVERIFY(content.contains("output"));
    QVERIFY(content.contains("assign"));

    QVERIFY(!codegen.inputMap().isEmpty());
    QVERIFY(!codegen.outputMap().isEmpty());
    QCOMPARE(codegen.moduleName(), QStringLiteral("and_test"));
    QVERIFY2(content.contains("module and_test ("),
             "The generated top-level module name must match moduleName()");
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
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
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    for (int i = 0; i < 4; ++i) {
        QVERIFY2(content.contains(QRegularExpression(QString("input \\S*_rot_%1").arg(i))),
                 qPrintable(QString("missing module input for rotary position %1").arg(i)));
    }
    QVERIFY2(!content.contains("aux_"), "rotary must not leave undriven aux wires");
}

void TestSystemVerilogCodeGenUnit::testDLatchAndSRLatchCircuits()
{
    // Neither the built-in DLatch nor SRLatch element is exercised by any
    // .panda-based integration fixture (those build latches structurally from
    // NAND/NOR gates), so assignVariablesRec's dedicated case for each type
    // is otherwise dead.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *swD = new InputSwitch;
    auto *swEnable = new InputSwitch;
    auto *dLatch = new DLatch;
    auto *dLatchQ = new Led;
    auto *dLatchQBar = new Led;

    auto *swS = new InputSwitch;
    auto *swR = new InputSwitch;
    auto *srLatch = new SRLatch;
    auto *srLatchQ = new Led;
    auto *srLatchQBar = new Led;

    builder.add(swD, swEnable, dLatch, dLatchQ, dLatchQBar,
                swS, swR, srLatch, srLatchQ, srLatchQBar);
    builder.connect(swD, 0, dLatch, 0);      // Data
    builder.connect(swEnable, 0, dLatch, 1); // Enable
    builder.connect(dLatch, 0, dLatchQ, 0);
    builder.connect(dLatch, 1, dLatchQBar, 0);

    builder.connect(swS, 0, srLatch, 0); // Set
    builder.connect(swR, 0, srLatch, 1); // Reset
    builder.connect(srLatch, 0, srLatchQ, 0);
    builder.connect(srLatch, 1, srLatchQBar, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/latches.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY(content.contains("//D Latch"));
    QVERIFY(content.contains("//End of D Latch"));
    QVERIFY(content.contains("//SR Latch"));
    QVERIFY(content.contains("//End of SR Latch"));
    // D Latch: transparent while Enable is high.
    QVERIFY2(content.contains(QRegularExpression(R"(if \(\S+\)\s*\n\s*begin)")),
             "D Latch must gate the pass-through on Enable");
    // SR Latch: S&&R both high is the invalid state, forcing both outputs low.
    QVERIFY2(content.contains(QRegularExpression(R"(if \(\S+ && \S+\))")),
             "SR Latch must guard the S&&R invalid state");
}

void TestSystemVerilogCodeGenUnit::testFlipFlopsNormalClockPath()
{
    // SRFlipFlop/JKFlipFlop/TFlipFlop's dedicated cases are otherwise dead for
    // the same reason as DLatch/SRLatch above. Preset/Clear are left
    // unconnected (required(false) + Active default resolves to a tied-off
    // "1'b1"), so this exercises each type's plain clock-edge body only.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *clk = new Clock;
    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;

    auto *srff = new SRFlipFlop;
    auto *jkff = new JKFlipFlop;
    auto *tff = new TFlipFlop;

    auto *srQ = new Led;
    auto *srQBar = new Led;
    auto *jkQ = new Led;
    auto *jkQBar = new Led;
    auto *tQ = new Led;
    auto *tQBar = new Led;

    builder.add(clk, sw1, sw2, srff, jkff, tff, srQ, srQBar, jkQ, jkQBar, tQ, tQBar);

    builder.connect(sw1, 0, srff, 0); // S
    builder.connect(clk, 0, srff, 1); // Clock
    builder.connect(sw2, 0, srff, 2); // R
    builder.connect(srff, 0, srQ, 0);
    builder.connect(srff, 1, srQBar, 0);

    builder.connect(sw1, 0, jkff, 0); // J
    builder.connect(clk, 0, jkff, 1); // Clock
    builder.connect(sw2, 0, jkff, 2); // K
    builder.connect(jkff, 0, jkQ, 0);
    builder.connect(jkff, 1, jkQBar, 0);

    builder.connect(sw1, 0, tff, 0); // T
    builder.connect(clk, 0, tff, 1); // Clock
    builder.connect(tff, 0, tQ, 0);
    builder.connect(tff, 1, tQBar, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/ffnormal.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY(content.contains("//SR FlipFlop"));
    QVERIFY(content.contains("//End of SR FlipFlop"));
    QVERIFY(content.contains("//JK FlipFlop"));
    QVERIFY(content.contains("//End of JK FlipFlop"));
    QVERIFY(content.contains("//T FlipFlop"));
    QVERIFY(content.contains("//End of T FlipFlop"));
    // None of the three has Preset/Clear wired — no async control may appear.
    QVERIFY2(!content.contains("negedge"), "Unwired Preset/Clear must not produce an async sensitivity entry");
}

void TestSystemVerilogCodeGenUnit::testFlipFlopsWithPresetAndClearWired()
{
    // No .panda fixture wires BOTH Preset and Clear on a flip-flop to a real
    // signal (only Clear, if anything, gets wired) — this drives both on
    // every edge-triggered type, exercising emitSequentialBlock's combined
    // sensitivity list, each type's own Preset/Clear body content, and the
    // "else if" Clear branch that only appears once Preset already claimed
    // the leading "if".
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *clk = new Clock;
    auto *data = new InputSwitch;
    auto *prst = new InputSwitch;
    auto *clr = new InputSwitch;

    auto *dff = new DFlipFlop;
    auto *tff = new TFlipFlop;
    auto *jkff = new JKFlipFlop;
    auto *srff = new SRFlipFlop;

    auto *dQ = new Led;
    auto *dQBar = new Led;
    auto *tQ = new Led;
    auto *tQBar = new Led;
    auto *jkQ = new Led;
    auto *jkQBar = new Led;
    auto *srQ = new Led;
    auto *srQBar = new Led;

    builder.add(clk, data, prst, clr, dff, tff, jkff, srff,
                dQ, dQBar, tQ, tQBar, jkQ, jkQBar, srQ, srQBar);

    builder.connect(data, 0, dff, 0);
    builder.connect(clk, 0, dff, 1);
    builder.connect(prst, 0, dff, 2);
    builder.connect(clr, 0, dff, 3);
    builder.connect(dff, 0, dQ, 0);
    builder.connect(dff, 1, dQBar, 0);

    builder.connect(data, 0, tff, 0);
    builder.connect(clk, 0, tff, 1);
    builder.connect(prst, 0, tff, 2);
    builder.connect(clr, 0, tff, 3);
    builder.connect(tff, 0, tQ, 0);
    builder.connect(tff, 1, tQBar, 0);

    builder.connect(data, 0, jkff, 0);
    builder.connect(clk, 0, jkff, 1);
    builder.connect(data, 0, jkff, 2);
    builder.connect(prst, 0, jkff, 3);
    builder.connect(clr, 0, jkff, 4);
    builder.connect(jkff, 0, jkQ, 0);
    builder.connect(jkff, 1, jkQBar, 0);

    builder.connect(data, 0, srff, 0);
    builder.connect(clk, 0, srff, 1);
    builder.connect(data, 0, srff, 2);
    builder.connect(prst, 0, srff, 3);
    builder.connect(clr, 0, srff, 4);
    builder.connect(srff, 0, srQ, 0);
    builder.connect(srff, 1, srQBar, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/ff_prst_clr.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY2(content.contains(QRegularExpression(R"(always @\(posedge \S+ or negedge \S+ or negedge \S+\))")),
             "Preset+Clear both wired must produce the 3-way sensitivity list");
    QVERIFY2(content.contains("        if (~"), "Preset guard must be emitted");
    QVERIFY2(content.contains("        else if (~"), "Clear must be an \"else if\" once Preset claimed the leading if");
    // Every one of the 4 types must emit its own Preset body (Q<=1, Qbar<=0) and
    // Clear body (Q<=0, Qbar<=1) — not just the shared control-flow scaffolding.
    QCOMPARE(content.count("//D FlipFlop"), 1);
    QCOMPARE(content.count("//T FlipFlop"), 1);
    QCOMPARE(content.count("//JK FlipFlop"), 1);
    QCOMPARE(content.count("//SR FlipFlop"), 1);
}

void TestSystemVerilogCodeGenUnit::testFlipFlopWithPresetOnlySensitivityList()
{
    // emitSequentialBlock's "Preset wired, Clear not" sensitivity list is a
    // distinct branch from both "Preset+Clear" and "Clear only" (the latter is
    // already exercised by real .panda register/counter fixtures).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *clk = new Clock;
    auto *data = new InputSwitch;
    auto *prst = new InputSwitch;
    auto *dff = new DFlipFlop;
    auto *q = new Led;
    auto *qBar = new Led;

    builder.add(clk, data, prst, dff, q, qBar);
    builder.connect(data, 0, dff, 0);
    builder.connect(clk, 0, dff, 1);
    builder.connect(prst, 0, dff, 2);
    // Clear (port 3) intentionally left unconnected — ties to its Active default.
    builder.connect(dff, 0, q, 0);
    builder.connect(dff, 1, qBar, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/dff_prst_only.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    // Only Preset is wired: exactly one "negedge" in the whole file.
    QCOMPARE(content.count("negedge"), 1);
    QVERIFY2(content.contains("        if (~"), "Preset body must still be reachable without Clear");
}

void TestSystemVerilogCodeGenUnit::testAsyncControlWithComplexExpressionCreatesAuxWire()
{
    // Preset driven by a NOT gate's inlined "~signal" expression, not a bare
    // signal name — invalid inside an "always @(... or negedge ...)" event
    // control, so ensureSimpleSignal() must hoist it into its own wire first.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *clk = new Clock;
    auto *data = new InputSwitch;
    auto *swPrstSrc = new InputSwitch;
    auto *notGate = new Not;
    auto *dff = new DFlipFlop;
    auto *q = new Led;
    auto *qBar = new Led;

    builder.add(clk, data, swPrstSrc, notGate, dff, q, qBar);
    builder.connect(data, 0, dff, 0);
    builder.connect(clk, 0, dff, 1);
    builder.connect(swPrstSrc, 0, notGate, 0);
    builder.connect(notGate, 0, dff, 2);
    builder.connect(dff, 0, q, 0);
    builder.connect(dff, 1, qBar, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/dff_complex_prst.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY2(content.contains(QRegularExpression(R"(wire aux_async_\d+ = ~\S+;)")),
             "A complex (non-identifier) async-control expression must be hoisted into its own wire");
    QVERIFY2(content.contains(QRegularExpression(R"(negedge aux_async_\d+)")),
             "The sensitivity list must reference the hoisted wire, not the raw expression");
}

void TestSystemVerilogCodeGenUnit::testOutOfTopologicalOrderGateResolution()
{
    // otherPortNameImpl()'s "otherPort's element is itself an inlineable gate,
    // recurse to compute its expression" fallback only fires when a downstream
    // gate is processed before the upstream gate feeding it — i.e. the element
    // list isn't topologically sorted. The elements vector is built directly
    // (bypassing Scene/CircuitBuilder, whose iteration order this test should
    // not depend on) so the order can be set explicitly and reproducibly.
    auto sw1 = std::make_unique<InputSwitch>();
    auto sw2 = std::make_unique<InputSwitch>();
    auto andGate = std::make_unique<And>();
    auto orGate = std::make_unique<Or>();
    auto led = std::make_unique<Led>();

    std::vector<std::unique_ptr<Connection>> conns;
    auto connect = [&](OutputPort *from, InputPort *to) {
        auto c = std::make_unique<Connection>();
        c->setStartPort(from);
        c->setEndPort(to);
        conns.push_back(std::move(c));
    };
    connect(sw1->outputPort(0), andGate->inputPort(0));
    connect(sw2->outputPort(0), andGate->inputPort(1));
    connect(andGate->outputPort(0), orGate->inputPort(0));
    connect(sw1->outputPort(0), orGate->inputPort(1));
    connect(orGate->outputPort(0), led->inputPort(0));

    // OR (downstream) listed before AND (its upstream driver): when
    // assignVariablesRec reaches OR, AND has not been inlined into m_varMap yet.
    QVector<GraphicElement *> elements{sw1.get(), sw2.get(), orGate.get(), andGate.get(), led.get()};

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/out_of_order.sv";

    SystemVerilogCodeGen codegen(path, elements);
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    const qsizetype outputSection = content.indexOf("// Writing output data. //");
    QVERIFY(outputSection >= 0);
    const QString outputCode = content.mid(outputSection);

    // The AND sub-expression must be inlined into the OR's assign, proving the
    // out-of-order recursive resolution worked instead of silently tying the
    // unresolved upstream gate off to a constant.
    QVERIFY2(outputCode.contains("&") && outputCode.contains("|"),
             "AND sub-expression must be inlined into the OR's assign");
    QVERIFY2(!outputCode.contains("1'b0"),
             "Out-of-order gate resolution must not fall back to a tied-off constant");
}

void TestSystemVerilogCodeGenUnit::testDeclareInputsWithNoOutputElements()
{
    // declareInputs()'s own totalOutputs tally only special-cases "no trailing
    // comma" for the truly last port in the whole module(...) list — which is
    // only the last INPUT when the circuit has no Output-group element at all.
    // Every other test in this file includes at least one Led.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw = new InputSwitch;
    builder.add(sw);

    QTemporaryDir dir;
    QString path = dir.path() + "/no_outputs.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    const qsizetype inputsStart = content.indexOf("/* ========= Inputs ========== */");
    const qsizetype outputsStart = content.indexOf("/* ========= Outputs ========== */");
    QVERIFY(inputsStart >= 0 && outputsStart > inputsStart);
    const QString inputsSection = content.mid(inputsStart, outputsStart - inputsStart);

    QVERIFY2(inputsSection.contains("input "), "Input declaration must still be emitted");
    QVERIFY2(!inputsSection.trimmed().endsWith(","),
             "The sole input must not get a trailing comma when no Output element exists");
}

void TestSystemVerilogCodeGenUnit::testDeclareOutputsWithLabel()
{
    // declareOutputs()'s label-suffix branch is only reached when the Output
    // element actually has a non-empty label; every earlier test uses an
    // unlabeled Led.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw = new InputSwitch;
    auto *led = new Led;
    led->setLabel("alarm");

    builder.add(sw, led);
    builder.connect(sw, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/labeled_output.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY2(content.contains(QRegularExpression("output \\S*alarm\\S*")),
             "Labeled output element must have its label folded into the generated port name");
}

void TestSystemVerilogCodeGenUnit::testDemuxWithFourOutputsUsesTwoSelectLines()
{
    // With only 2 outputs (the existing testDemuxCircuit), numSelectLines'
    // while-loop condition ((1<<1)=2 < 2) is false immediately and never
    // increments — 4 outputs forces at least one real iteration.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *swData = new InputSwitch;
    auto *swSel0 = new InputSwitch;
    auto *swSel1 = new InputSwitch;
    auto *demux = new Demux;
    demux->setOutputSize(4);

    builder.add(swData, swSel0, swSel1, demux);
    builder.connect(swData, 0, demux, 0); // data
    builder.connect(swSel0, 0, demux, 1); // select bit 0
    builder.connect(swSel1, 0, demux, 2); // select bit 1

    QVector<Led *> leds(4);
    for (int i = 0; i < 4; ++i) {
        leds[i] = new Led;
        builder.add(leds[i]);
        builder.connect(demux, i, leds[i], 0);
    }

    QTemporaryDir dir;
    QString path = dir.path() + "/demux4.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY2(content.contains("2'd0:"), "4-output Demux needs 2 select lines (2'd-prefixed case labels)");
    QVERIFY2(content.contains("2'd3:"), "All 4 case values must be emitted");
}

void TestSystemVerilogCodeGenUnit::testIcPortNamesEscapeReservedWordsAndDedup()
{
    // collectICTypes()'s reserved-word escape and dedup-suffix logic for IC
    // boundary port names has no exercising fixture: none of the real .panda
    // building blocks happen to use a SystemVerilog reserved word or a
    // duplicate label. Round-tripping a hand-built sub-circuit through a real
    // save+load (exactly how a user creates an IC) gives full control over the
    // boundary labels without hand-crafting a .panda blob.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    WorkSpace subWorkspace;
    CircuitBuilder subBuilder(subWorkspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    sw2->setLabel("wire"); // SystemVerilog reserved word -> escaped "p_wire"
    auto *sw3 = new InputSwitch;
    sw3->setLabel("sig");
    auto *sw4 = new InputSwitch;
    sw4->setLabel("sig"); // duplicate label -> dedup suffix "sig_1"

    auto *andGate = new And;
    auto *orGate = new Or;

    auto *ledMain = new Led;
    auto *ledB = new Led;
    ledB->setLabel("res");
    auto *ledC = new Led;
    ledC->setLabel("res"); // duplicate label -> dedup suffix "res_1"

    subBuilder.add(sw1, sw2, sw3, sw4, andGate, orGate, ledMain, ledB, ledC);
    subBuilder.connect(sw1, 0, andGate, 0);
    subBuilder.connect(sw2, 0, andGate, 1);
    subBuilder.connect(sw3, 0, orGate, 0);
    subBuilder.connect(sw4, 0, orGate, 1);
    subBuilder.connect(andGate, 0, ledMain, 0);
    subBuilder.connect(orGate, 0, ledB, 0);
    subBuilder.connect(andGate, 0, ledC, 0);

    const QString subPath = subDir.path() + "/reserved_dedup_sub.panda";
    QCOMPARE(subWorkspace.save(subPath), WorkSpace::SaveOutcome::Saved);

    auto ic = std::make_unique<IC>();
    ic->loadFile(subPath, subDir.path());
    QCOMPARE(ic->inputSize(), 4);
    QCOMPARE(ic->outputSize(), 3);

    auto swA = std::make_unique<InputSwitch>();
    auto swB = std::make_unique<InputSwitch>();
    auto swC = std::make_unique<InputSwitch>();
    auto swD = std::make_unique<InputSwitch>();

    std::vector<std::unique_ptr<Connection>> conns;
    auto connect = [&](OutputPort *from, InputPort *to) {
        auto c = std::make_unique<Connection>();
        c->setStartPort(from);
        c->setEndPort(to);
        conns.push_back(std::move(c));
    };
    connect(swA->outputPort(0), ic->inputPort(0));
    connect(swB->outputPort(0), ic->inputPort(1));
    connect(swC->outputPort(0), ic->inputPort(2));
    connect(swD->outputPort(0), ic->inputPort(3));

    QVector<GraphicElement *> elements{swA.get(), swB.get(), swC.get(), swD.get(), ic.get()};

    QTemporaryDir outDir;
    QVERIFY(outDir.isValid());
    QString svPath = outDir.path() + "/ic_reserved.sv";
    SystemVerilogCodeGen codegen(svPath, elements);
    codegen.generate();

    QFile file(svPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY2(content.contains("p_wire"), "A reserved-word port label must be escaped with a p_ prefix");
    QVERIFY2(content.contains(QRegularExpression(R"(\bsig\b)")) && content.contains("sig_1"),
             "Duplicate port labels must be deduplicated with a numeric suffix");
    QVERIFY2(content.contains(QRegularExpression(R"(\bres\b)")) && content.contains("res_1"),
             "Duplicate output labels must be deduplicated with a numeric suffix too");
}

void TestSystemVerilogCodeGenUnit::testOutOfOrderNodeUpstreamResolution()
{
    // otherPortNameImpl()'s inlineable-gate check is a multi-way "||" over 8
    // element types; the out-of-order AND/OR test above only exercises the
    // first (And) disjunct via short-circuit. A Node upstream driver forces
    // evaluation all the way through the chain to its own (last) disjunct.
    auto sw1 = std::make_unique<InputSwitch>();
    auto sw2 = std::make_unique<InputSwitch>();
    auto node = std::make_unique<Node>();
    auto andGate = std::make_unique<And>();
    auto led = std::make_unique<Led>();

    std::vector<std::unique_ptr<Connection>> conns;
    auto connect = [&](OutputPort *from, InputPort *to) {
        auto c = std::make_unique<Connection>();
        c->setStartPort(from);
        c->setEndPort(to);
        conns.push_back(std::move(c));
    };
    connect(sw1->outputPort(0), node->inputPort(0));
    connect(node->outputPort(0), andGate->inputPort(0));
    connect(sw2->outputPort(0), andGate->inputPort(1));
    connect(andGate->outputPort(0), led->inputPort(0));

    // AND (downstream) listed before Node (its upstream driver).
    QVector<GraphicElement *> elements{sw1.get(), sw2.get(), andGate.get(), node.get(), led.get()};

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/node_out_of_order.sv";

    SystemVerilogCodeGen codegen(path, elements);
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    const qsizetype outputSection = content.indexOf("// Writing output data. //");
    QVERIFY(outputSection >= 0);
    const QString outputCode = content.mid(outputSection);

    QVERIFY2(outputCode.contains("&"),
             "The Node's pass-through value must still be inlined into the AND expression");
    QVERIFY2(!outputCode.contains("1'b0"),
             "Out-of-order Node resolution must not fall back to a tied-off constant");
}

void TestSystemVerilogCodeGenUnit::testWirelessNodeCycleDoesNotInfiniteRecurse()
{
    // otherPortNameImpl()'s `visited` set exists purely to survive a
    // combinational cycle without infinite recursion / a stack overflow. Two
    // Nodes wired directly into each other (no real circuit could be drawn
    // this way through the UI, but nothing at the codegen level guards
    // against a malformed/hand-built graph) forces the cycle-detection
    // branch to actually fire.
    auto n1 = std::make_unique<Node>();
    auto n2 = std::make_unique<Node>();
    auto led = std::make_unique<Led>();

    std::vector<std::unique_ptr<Connection>> conns;
    auto connect = [&](OutputPort *from, InputPort *to) {
        auto c = std::make_unique<Connection>();
        c->setStartPort(from);
        c->setEndPort(to);
        conns.push_back(std::move(c));
    };
    connect(n2->outputPort(0), n1->inputPort(0));
    connect(n1->outputPort(0), n2->inputPort(0));
    connect(n1->outputPort(0), led->inputPort(0));

    QVector<GraphicElement *> elements{n1.get(), n2.get(), led.get()};

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/node_cycle.sv";

    SystemVerilogCodeGen codegen(path, elements);
    try {
        codegen.generate();
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("SystemVerilog generation threw for a cyclic Node graph: %1").arg(e.what())));
    }

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY2(content.contains("module") && content.contains("endmodule"),
             "Generation must terminate and still produce a structurally complete module despite the cycle");
}

void TestSystemVerilogCodeGenUnit::testNandGateCircuit()
{
    // None of the other tests in this file (or the .panda-based integration
    // fixtures) happen to place a raw Nand element directly, leaving
    // generateLogicExpressionImpl()'s dedicated Nand case (and its "negate the
    // parenthesized multi-input expression" path) untested.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw1 = new InputSwitch;
    auto *sw2 = new InputSwitch;
    auto *nandGate = new Nand;
    auto *led = new Led;

    builder.add(sw1, sw2, nandGate, led);
    builder.connect(sw1, 0, nandGate, 0);
    builder.connect(sw2, 0, nandGate, 1);
    builder.connect(nandGate, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/nand_test.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    QVERIFY2(content.contains(QRegularExpression(R"(assign \S+ = ~\(\S+ & \S+\);)")),
             "NAND must emit a negated, parenthesized AND expression");
}

void TestSystemVerilogCodeGenUnit::testDoubleNotCancelsNegation()
{
    // generateLogicExpressionImpl()'s Not case cancels a double negation
    // (~~x -> x) instead of emitting a redundant double tilde; only a NOT
    // feeding directly into another NOT exercises that cancellation.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *sw = new InputSwitch;
    auto *not1 = new Not;
    auto *not2 = new Not;
    auto *led = new Led;

    builder.add(sw, not1, not2, led);
    builder.connect(sw, 0, not1, 0);
    builder.connect(not1, 0, not2, 0);
    builder.connect(not2, 0, led, 0);

    QTemporaryDir dir;
    QString path = dir.path() + "/double_not.sv";
    SystemVerilogCodeGen codegen(path, sceneElements(workspace.scene()));
    codegen.generate();

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = file.readAll();

    const qsizetype outputSection = content.indexOf("// Writing output data. //");
    QVERIFY(outputSection >= 0);
    const QString outputCode = content.mid(outputSection);

    QVERIFY2(!outputCode.contains("~~"),
             "A NOT feeding directly into another NOT must cancel to the bare signal, not double-negate");
}
