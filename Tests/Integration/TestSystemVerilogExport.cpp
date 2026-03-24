// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestSystemVerilogExport.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRandomGenerator>
#include <QTemporaryFile>
#include <QTextStream>

#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

// --- Sequential IC detection and testbench support ---

enum class SequentialICType { None, SRLatch, DLatch, DFlipFlop, JKFlipFlop };

static SequentialICType detectSequentialType(IC *ic)
{
    if (ic->outputSize() != 2) {
        return SequentialICType::None;
    }
    QString out0 = ic->outputPort(0)->name().toLower().trimmed();
    QString out1 = ic->outputPort(1)->name().toLower().trimmed();
    if (out0 != "q" || out1 != "q_bar") {
        return SequentialICType::None;
    }

    const int n = ic->inputSize();
    auto portName = [&](int i) { return ic->inputPort(i)->name().toLower().trimmed(); };

    if (n == 2 && portName(0) == "s" && portName(1) == "r") {
        return SequentialICType::SRLatch;
    }
    if (n == 2 && portName(0) == "d" && portName(1) == "enable") {
        return SequentialICType::DLatch;
    }
    if (n == 4 && portName(0) == "d" && portName(1) == "clock"
        && portName(2) == "preset" && portName(3) == "clear") {
        return SequentialICType::DFlipFlop;
    }
    if (n == 5 && portName(0) == "j" && portName(1) == "k" && portName(2) == "clock"
        && portName(3) == "preset" && portName(4) == "clear") {
        return SequentialICType::JKFlipFlop;
    }

    return SequentialICType::None;
}

// --- Generic sequential IC detection for categories A-F ---

struct PortRoles {
    int clockIdx = -1;
    int resetIdx = -1;      // active-HIGH reset
    int presetIdx = -1;     // active-LOW preset
    int weIdx = -1;         // write enable
    int loadIdx = -1;       // parallel load
    int enableIdx = -1;     // count/operation enable
    int directionIdx = -1;  // up/down direction
    int sinIdx = -1;        // serial input
    QVector<int> dataIdxs;  // everything else (data/address buses)
};

static PortRoles detectPortRoles(IC *ic)
{
    PortRoles roles;
    for (int i = 0; i < ic->inputSize(); ++i) {
        const QString name = ic->inputPort(i)->name().toLower().trimmed();
        if (name == "clock" || name == "clk") {
            roles.clockIdx = i;
        } else if (name == "reset" || name == "sp_reset") {
            roles.resetIdx = i;
        } else if (name == "preset") {
            roles.presetIdx = i;
        } else if (name == "writeenable" || name == "write_enable") {
            roles.weIdx = i;
        } else if (name == "load") {
            roles.loadIdx = i;
        } else if (name == "enable") {
            roles.enableIdx = i;
        } else if (name == "direction") {
            roles.directionIdx = i;
        } else if (name == "sin") {
            roles.sinIdx = i;
        } else {
            roles.dataIdxs.append(i);
        }
    }
    return roles;
}

struct SeqTestStep {
    QVector<int> inputBits;    // input values for this step
    QVector<int> expectedOut;  // expected output values (empty = don't check)
    QString comment;           // description for testbench comments
};

static void setInputAndSettle(const QVector<InputSwitch *> &switches, Simulation *sim, int index, int value)
{
    switches[index]->setOn(value != 0);
    for (int i = 0; i < 20; ++i) {
        sim->update();
    }
}

static QVector<int> readOutputs(const QVector<Led *> &leds)
{
    QVector<int> result;
    result.reserve(leds.size());
    for (auto *led : leds) {
        result.append(TestUtils::getInputStatus(led, 0) ? 1 : 0);
    }
    return result;
}

static QVector<int> captureInputs(const QVector<InputSwitch *> &switches)
{
    QVector<int> bits;
    bits.reserve(switches.size());
    for (auto *sw : switches) {
        bits.append(sw->isOn() ? 1 : 0);
    }
    return bits;
}

// Record a step that checks outputs
static void recordCheck(QVector<SeqTestStep> &steps, const QVector<InputSwitch *> &switches,
                         const QVector<Led *> &leds, const QString &comment = {})
{
    steps.append({captureInputs(switches), readOutputs(leds), comment});
}

// Record a setup step (no output check — just drives inputs for transitions)
static void recordSetup(QVector<SeqTestStep> &steps, const QVector<InputSwitch *> &switches,
                         const QString &comment = {})
{
    steps.append({captureInputs(switches), {}, comment});
}

static QVector<SeqTestStep> collectSequentialVectors(
    SequentialICType seqType,
    const QVector<InputSwitch *> &switches,
    const QVector<Led *> &leds,
    Simulation *sim)
{
    QVector<SeqTestStep> steps;

    auto setInput = [&](int index, int value) {
        setInputAndSettle(switches, sim, index, value);
    };

    switch (seqType) {
    case SequentialICType::SRLatch: {
        // inputs: [0]=s, [1]=r
        // Step: Set (s=1, r=0)
        setInput(0, 1);
        setInput(1, 0);
        recordCheck(steps, switches, leds, "Set");

        // Step: Hold (s=0, r=0)
        setInput(0, 0);
        recordCheck(steps, switches, leds, "Hold after set");

        // Step: Reset (s=0, r=1)
        setInput(1, 1);
        recordCheck(steps, switches, leds, "Reset");

        // Step: Hold (s=0, r=0)
        setInput(1, 0);
        recordCheck(steps, switches, leds, "Hold after reset");

        // Step: Forbidden (s=1, r=1)
        setInput(0, 1);
        setInput(1, 1);
        recordCheck(steps, switches, leds, "Forbidden");
        break;
    }

    case SequentialICType::DLatch: {
        // inputs: [0]=d, [1]=enable
        // Enable=1, D=0 → transparent
        setInput(1, 1);
        setInput(0, 0);
        recordCheck(steps, switches, leds, "Transparent D=0");

        // Enable=1, D=1 → transparent
        setInput(0, 1);
        recordCheck(steps, switches, leds, "Transparent D=1");

        // Enable=0, D=0 → hold
        setInput(1, 0);
        setInput(0, 0);
        recordCheck(steps, switches, leds, "Hold after D=1");

        // Enable=0, D=1 → hold
        setInput(0, 1);
        recordCheck(steps, switches, leds, "Hold D changes ignored");

        // Enable=1, D=0 → transparent again
        setInput(1, 1);
        setInput(0, 0);
        recordCheck(steps, switches, leds, "Transparent D=0 again");
        break;
    }

    case SequentialICType::DFlipFlop: {
        // inputs: [0]=d, [1]=clock, [2]=preset, [3]=clear
        // Gate-level master-slave: clock HIGH captures master, LOW latches slave.

        auto clockPulseDFF = [&]() {
            switches[1]->setOn(true);   // clock HIGH
            for (int u = 0; u < 20; ++u) sim->update();
            recordSetup(steps, switches, "Clock HIGH");
            switches[1]->setOn(false);  // clock LOW
            for (int u = 0; u < 20; ++u) sim->update();
        };

        // Initialize: preset=1, clear=1, clock=0, d=0
        switches[0]->setOn(false);  // d=0
        switches[1]->setOn(false);  // clock=0
        switches[2]->setOn(true);   // preset=1 (inactive)
        switches[3]->setOn(true);   // clear=1 (inactive)
        for (int u = 0; u < 20; ++u) sim->update();

        // D=0, clock pulse → initial state Q=0
        recordSetup(steps, switches, "Init D=0 clock=0");
        clockPulseDFF();
        recordCheck(steps, switches, leds, "Init Q=0");

        // D=1, clock pulse → capture Q=1
        switches[0]->setOn(true);   // d=1
        for (int u = 0; u < 20; ++u) sim->update();
        recordSetup(steps, switches, "Setup D=1");
        clockPulseDFF();
        recordCheck(steps, switches, leds, "Capture D=1");

        // D=0, clock pulse → capture Q=0
        switches[0]->setOn(false);  // d=0
        for (int u = 0; u < 20; ++u) sim->update();
        recordSetup(steps, switches, "Setup D=0");
        clockPulseDFF();
        recordCheck(steps, switches, leds, "Capture D=0");

        // Async preset: preset=0 → Q=1
        switches[2]->setOn(false);
        for (int u = 0; u < 20; ++u) sim->update();
        recordCheck(steps, switches, leds, "Async preset");

        // Release preset, D=0, clock pulse → Q=0
        switches[2]->setOn(true);
        switches[0]->setOn(false);
        for (int u = 0; u < 20; ++u) sim->update();
        recordSetup(steps, switches, "Setup D=0 after preset");
        clockPulseDFF();
        recordCheck(steps, switches, leds, "Capture D=0 after preset");

        // Async clear: clear=0 → Q=0
        switches[3]->setOn(false);
        for (int u = 0; u < 20; ++u) sim->update();
        recordCheck(steps, switches, leds, "Async clear");
        break;
    }

    case SequentialICType::JKFlipFlop: {
        // inputs: [0]=j, [1]=k, [2]=clock, [3]=preset, [4]=clear
        // Gate-level master-slave: clock HIGH captures in master, LOW latches slave.
        // Following unit test pattern: minimal sim->update() per step.

        // Helper: clock pulse matching the working unit test pattern.
        // Records both the rising edge (setup) and falling edge states.
        auto clockPulseJK = [&]() {
            switches[2]->setOn(true);   // clock HIGH
            sim->update();
            recordSetup(steps, switches, "Clock HIGH");
            switches[2]->setOn(false);  // clock LOW
            sim->update();
        };

        // Initialize: preset=1, clear=1, clock=0, j=0, k=0
        switches[0]->setOn(false);
        switches[1]->setOn(false);
        switches[2]->setOn(false);
        switches[3]->setOn(true);
        switches[4]->setOn(true);
        sim->update();

        // Reset to known state: J=0, K=1, clock pulse → Q=0
        switches[0]->setOn(false);  // j=0
        switches[1]->setOn(true);   // k=1
        recordSetup(steps, switches, "Init reset J=0 K=1");
        clockPulseJK();
        recordCheck(steps, switches, leds, "Init reset Q=0");

        // Set: J=1, K=0, clock pulse → Q=1
        switches[0]->setOn(true);   // j=1
        switches[1]->setOn(false);  // k=0
        recordSetup(steps, switches, "Setup J=1 K=0");
        clockPulseJK();
        recordCheck(steps, switches, leds, "Set J=1 K=0");

        // Reset: J=0, K=1, clock pulse → Q=0
        switches[0]->setOn(false);  // j=0
        switches[1]->setOn(true);   // k=1
        recordSetup(steps, switches, "Setup J=0 K=1");
        clockPulseJK();
        recordCheck(steps, switches, leds, "Reset J=0 K=1");

        // Toggle: J=1, K=1, clock pulse → Q=1
        switches[0]->setOn(true);   // j=1
        switches[1]->setOn(true);   // k=1
        recordSetup(steps, switches, "Setup J=1 K=1");
        clockPulseJK();
        recordCheck(steps, switches, leds, "Toggle J=1 K=1");

        // Toggle again: J=1, K=1, clock pulse → Q=0
        recordSetup(steps, switches, "Setup toggle again");
        clockPulseJK();
        recordCheck(steps, switches, leds, "Toggle again");

        // Hold: J=0, K=0, clock pulse → Q=0
        switches[0]->setOn(false);  // j=0
        switches[1]->setOn(false);  // k=0
        recordSetup(steps, switches, "Setup J=0 K=0");
        clockPulseJK();
        recordCheck(steps, switches, leds, "Hold J=0 K=0");

        // Async preset: preset=0 → Q=1
        switches[3]->setOn(false);
        sim->update();
        recordCheck(steps, switches, leds, "Async preset");
        break;
    }

    default:
        break;
    }

    return steps;
}

static QVector<SeqTestStep> collectGenericSequentialVectors(
    const PortRoles &roles,
    const QVector<InputSwitch *> &switches,
    const QVector<Led *> &leds,
    Simulation *sim)
{
    QVector<SeqTestStep> steps;
    const int clockIdx = roles.clockIdx;

    // Helper: settle simulation
    auto settle = [&]() {
        for (int u = 0; u < 20; ++u) sim->update();
    };

    // Helper: clock pulse (HIGH + settle + LOW + settle)
    // Both phases are recorded so the testbench generates proper 0→1→0 transitions.
    auto clockPulse = [&]() {
        switches[clockIdx]->setOn(true);
        settle();
        recordSetup(steps, switches, "Clock HIGH");
        switches[clockIdx]->setOn(false);
        settle();
        recordSetup(steps, switches, "Clock LOW");
    };

    // Initialize: all inputs to 0
    for (auto *sw : switches) {
        sw->setOn(false);
    }
    settle();

    // Phase 1: establish known state
    if (roles.resetIdx >= 0) {
        // Assert active-HIGH reset, pulse clock twice, deassert, pulse once
        switches[roles.resetIdx]->setOn(true);
        settle();
        clockPulse();
        clockPulse();
        switches[roles.resetIdx]->setOn(false);
        settle();
        recordSetup(steps, switches, "Reset deasserted");
        clockPulse();
    } else if (roles.presetIdx >= 0) {
        // Active-LOW preset: assert (LOW), pulse, deassert (HIGH), pulse
        switches[roles.presetIdx]->setOn(false); // already 0, but explicit
        settle();
        clockPulse();
        switches[roles.presetIdx]->setOn(true);
        settle();
        recordSetup(steps, switches, "Preset deasserted");
        clockPulse();
    } else {
        // No reset/preset: flush with clock pulses
        for (int f = 0; f < 4; ++f) {
            clockPulse();
        }
    }

    // Record initial output state
    recordCheck(steps, switches, leds, "Initial state after reset");

    // Phase 2: sweep data patterns with clock
    const int numDataBits = static_cast<int>(roles.dataIdxs.size());
    QVector<int> patterns;

    if (numDataBits == 0) {
        // No data bits (e.g., counter with just clock+reset) — just clock several times
        for (int c = 0; c < 8; ++c) {
            recordSetup(steps, switches, QString("Clock cycle %1").arg(c));
            clockPulse();
            recordCheck(steps, switches, leds, QString("After clock %1").arg(c));
        }
    } else {
        if (numDataBits <= 8) {
            // Exhaustive
            for (int v = 0; v < (1 << numDataBits); ++v) {
                patterns << v;
            }
        } else {
            // Sampled: 0, all-1s, walking-1s, random fill to 32
            patterns << 0 << ((1 << numDataBits) - 1);
            for (int i = 0; i < numDataBits; ++i) {
                patterns << (1 << i);
            }
            while (patterns.size() < 32) {
                patterns << QRandomGenerator::global()->bounded(1 << numDataBits);
            }
        }

        // Enable write if applicable
        auto enableWrite = [&]() {
            if (roles.weIdx >= 0) switches[roles.weIdx]->setOn(true);
            if (roles.loadIdx >= 0) switches[roles.loadIdx]->setOn(true);
            if (roles.enableIdx >= 0) switches[roles.enableIdx]->setOn(true);
        };
        auto disableWrite = [&]() {
            if (roles.weIdx >= 0) switches[roles.weIdx]->setOn(false);
            if (roles.loadIdx >= 0) switches[roles.loadIdx]->setOn(false);
        };

        for (int p = 0; p < patterns.size(); ++p) {
            const int pattern = patterns[p];

            // Set data inputs
            for (int b = 0; b < numDataBits; ++b) {
                switches[roles.dataIdxs[b]]->setOn((pattern >> b) & 1);
            }
            enableWrite();
            settle();

            recordSetup(steps, switches, QString("Setup pattern %1").arg(p));
            clockPulse();
            recordCheck(steps, switches, leds, QString("Pattern %1").arg(p));

            // Hold test: deassert write, clock, verify outputs stay
            if (roles.weIdx >= 0 || roles.loadIdx >= 0) {
                disableWrite();
                settle();
                recordSetup(steps, switches, QString("Hold test %1").arg(p));
                clockPulse();
                recordCheck(steps, switches, leds, QString("Hold verify %1").arg(p));
            }
        }
    }

    return steps;
}

void TestSystemVerilogExport::initTestCase()
{
#ifndef Q_OS_LINUX
    QSKIP("SystemVerilog export tests require iverilog/yosys/verilator (Linux CI only)");
#else
    TestUtils::setupTestEnvironment();
#endif
}

void TestSystemVerilogExport::cleanupTestCase()
{
    // Cleanup after all tests
}

void TestSystemVerilogExport::testWirelessNodeGeneration()
{
    // A Tx and Rx node pair on the same channel. In SystemVerilog, node expressions
    // are inlined, so the Rx node's expression should resolve to the InputSwitch
    // signal rather than the default 1'b0.
    auto *sw = new InputSwitch();
    auto *txNode = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(txNode);
    txNode->setLabel("CLK");
    txNode->setWirelessMode(WirelessMode::Tx);

    auto *rxNode = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(rxNode);
    rxNode->setLabel("CLK");
    rxNode->setWirelessMode(WirelessMode::Rx);

    auto *led = new Led();

    // Physical wire: InputSwitch → Tx node, Rx node → LED
    auto *swOut = sw->outputPort(0);
    auto *txIn = txNode->inputPort(0);
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(swOut);
    conn1->setEndPort(txIn);

    auto *rxOut = rxNode->outputPort(0);
    auto *ledIn = led->inputPort(0);
    auto *conn2 = new QNEConnection();
    conn2->setStartPort(rxOut);
    conn2->setEndPort(ledIn);

    QVector<GraphicElement *> elements{sw, txNode, rxNode, led};

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString verilogPath = tempDir.filePath("test_wireless.sv");

    SystemVerilogCodeGen generator(verilogPath, elements);
    try {
        generator.generate();
    } catch (...) {
        QFAIL("SystemVerilog generation threw an exception for wireless circuit");
    }

    QFile file(verilogPath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Generated SystemVerilog file should be readable");
    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(!content.isEmpty(), "Generated SystemVerilog should not be empty");

    // The output assignment section has the format:
    //   assign <output_var> = <expr>;
    // With wireless resolution, <expr> should be the InputSwitch signal name, not 1'b0.
    const qsizetype outputSection = content.indexOf("// Writing output data. //");
    QVERIFY2(outputSection >= 0, "Generated SV should contain output data section");
    const QString outputCode = content.mid(outputSection);

    QRegularExpression outputLow(R"(assign\s+\w+\s*=\s*1'b0;)");
    QVERIFY2(!outputLow.match(outputCode).hasMatch(),
             "Wireless Rx node should not result in 1'b0 in the LED output assignment");

    delete conn1;
    delete conn2;
    delete sw;
    delete txNode;
    delete rxNode;
    delete led;
}

void TestSystemVerilogExport::testWirelessOrphanedRxCodegen()
{
    // An Rx node with no matching Tx must not crash SystemVerilog code generation.
    auto *rxNode = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(rxNode);
    rxNode->setLabel("MISSING_TX");
    rxNode->setWirelessMode(WirelessMode::Rx);

    auto *led = new Led();

    auto *conn = new QNEConnection();
    conn->setStartPort(rxNode->outputPort(0));
    conn->setEndPort(led->inputPort(0));

    QVector<GraphicElement *> elements{rxNode, led};

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString verilogPath = tempDir.filePath("test_orphan_rx.sv");

    SystemVerilogCodeGen generator(verilogPath, elements);
    try {
        generator.generate();
    } catch (...) {
        QFAIL("SystemVerilog generation threw an exception for orphaned Rx circuit");
    }

    QFile file(verilogPath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Generated SystemVerilog file should be readable");
    const QString content = file.readAll();
    QVERIFY2(!content.isEmpty(), "Generated SystemVerilog should not be empty");

    delete conn;
    delete rxNode;
    delete led;
}

bool TestSystemVerilogExport::validateWithIverilog(const QString &verilogFile)
{
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("iverilog", QStringList{"-g2012", "-o", "/dev/null", verilogFile});

    if (!process.waitForFinished(5000)) {
        process.kill();
        return false;
    }

    return (process.exitCode() == 0);
}

bool TestSystemVerilogExport::validateWithYosys(const QString &verilogFile, const QString &moduleName)
{
    QString modName = moduleName;
    if (modName.isEmpty()) {
        QFileInfo fileInfo(verilogFile);
        modName = fileInfo.completeBaseName();
    }

    QStringList commands;
    commands << "-p"
             << QString("read_verilog -sv %1; synth -top %2").arg(verilogFile, modName);

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("yosys", commands);

    if (!process.waitForFinished(5000)) {
        process.kill();
        return false;
    }

    return (process.exitCode() == 0);
}

bool TestSystemVerilogExport::validateWithVerilator(const QString &verilogFile)
{
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("verilator", QStringList{"--lint-only", verilogFile});

    if (!process.waitForFinished(5000)) {
        process.kill();
        return false;
    }

    return (process.exitCode() == 0);
}

bool TestSystemVerilogExport::validateWithTestbench(const QString &verilogFile,
                                              const QString &tbFile)
{
    QString simOut = verilogFile + ".sim";

    QProcess compile;
    compile.setProcessChannelMode(QProcess::MergedChannels);
    compile.start("iverilog", {"-g2012", "-o", simOut, tbFile, verilogFile});

    if (!compile.waitForFinished(30000) || compile.exitCode() != 0) {
        qWarning() << "Testbench compilation failed:" << compile.readAll();
        QFile::remove(simOut);
        return false;
    }

    QProcess run;
    run.setProcessChannelMode(QProcess::MergedChannels);
    run.start("vvp", {simOut});

    if (!run.waitForFinished(30000)) {
        run.kill();
        QFile::remove(simOut);
        return false;
    }

    QString output = QString::fromUtf8(run.readAllStandardOutput());
    QFile::remove(simOut);

    return output.contains("PASS") && !output.contains("FAIL");
}

void TestSystemVerilogExport::testSystemVerilogExportHelper(const QString &icFile)
{
    // Create workspace and circuit builder
    std::unique_ptr<WorkSpace> workspace = TestUtils::createWorkspace();
    QVERIFY(workspace != nullptr);

    auto *scene = workspace->scene();
    QVERIFY(scene != nullptr);

    // Create circuit builder
    CircuitBuilder builder(scene);

    // Load IC from test components directory
    IC *ic = nullptr;
    try {
        ic = CPUTestUtils::loadBuildingBlockIC(icFile);
    } catch (const std::exception &ex) {
        QFAIL(qPrintable(QString("Failed to load IC: %1").arg(QString::fromStdString(ex.what()))));
        return;
    }

    builder.add(ic);

    // Create input switches (one per IC input)
    QVector<InputSwitch *> switches;
    for (int i = 0; i < ic->inputSize(); ++i) {
        auto *sw = new InputSwitch();
        builder.add(sw);
        switches.append(sw);
        builder.connect(sw, 0, ic, i);
    }

    // Create LEDs (one per IC output)
    QVector<Led *> leds;
    for (int i = 0; i < ic->outputSize(); ++i) {
        auto *led = new Led();
        builder.add(led);
        leds.append(led);
        builder.connect(ic, i, led, 0);
    }

    // Initialize simulation
    Simulation *sim = nullptr;
    try {
        sim = builder.initSimulation();
    } catch (const std::exception &ex) {
        QFAIL(qPrintable(QString("Simulation initialization failed: %1").arg(QString::fromStdString(ex.what()))));
        return;
    }
    QVERIFY(sim != nullptr);

    // Detect sequential IC type for clock-aware testbenches
    SequentialICType seqType = detectSequentialType(ic);

    // Get all elements from scene
    QVector<GraphicElement *> allElements;
    for (auto *item : scene->items()) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            allElements.append(elm);
        }
    }

    QVERIFY2(!allElements.isEmpty(), "Scene should contain elements");

    // Create SystemVerilog file in a persistent debug directory
    QDir debugDir = QDir::home();
    debugDir.mkpath(".wiredpanda_systemverilog_debug");
    debugDir.cd(".wiredpanda_systemverilog_debug");

    // Use IC filename for easier debugging
    QFileInfo icInfo(icFile);
    QString verilogPath = debugDir.filePath(icInfo.baseName() + ".sv");

    // Generate SystemVerilog code
    SystemVerilogCodeGen generator(verilogPath, allElements);
    try {
        generator.generate();
    } catch (const std::exception &ex) {
        QFile::remove(verilogPath);
        QFAIL(qPrintable(QString("SystemVerilog generation failed: %1").arg(QString::fromStdString(ex.what()))));
        return;
    } catch (...) {
        QFile::remove(verilogPath);
        QFAIL("SystemVerilog generation threw unknown exception");
        return;
    }

    // Verify file was created
    QVERIFY2(QFileInfo::exists(verilogPath), "SystemVerilog file should be created");

    // Validate with tools
    QVERIFY2(validateWithIverilog(verilogPath), qPrintable(QString("iverilog validation failed for %1").arg(icFile)));
    QVERIFY2(validateWithYosys(verilogPath), qPrintable(QString("yosys validation failed for %1").arg(icFile)));
    QVERIFY2(validateWithVerilator(verilogPath), qPrintable(QString("verilator validation failed for %1").arg(icFile)));

    const auto &inputPins = generator.inputMap();
    const auto &outputPins = generator.outputMap();
    const QString &modName = generator.moduleName();

    // Collect sequential test vectors (Level 1 dedicated or generic)
    QVector<SeqTestStep> seqSteps;
    if (seqType != SequentialICType::None) {
        seqSteps = collectSequentialVectors(seqType, switches, leds, sim);
    } else {
        PortRoles roles = detectPortRoles(ic);
        if (roles.clockIdx >= 0) {
            seqSteps = collectGenericSequentialVectors(roles, switches, leds, sim);
        }
    }

    if (!seqSteps.isEmpty()) {
        // --- Sequential testbench: clock-aware multi-step validation ---
        QString tbPath = debugDir.filePath(icInfo.baseName() + "_tb.sv");
        QFile tbFile(tbPath);
        if (tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream tb(&tbFile);
            tb << "`timescale 1ns/1ps\n";
            tb << "module testbench;\n";

            for (const auto &pin : inputPins) {
                tb << "    reg " << pin.m_varName << ";\n";
            }
            for (const auto &pin : outputPins) {
                tb << "    wire " << pin.m_varName << ";\n";
            }
            tb << "    integer errors;\n\n";

            // Instantiate DUT
            tb << "    " << modName << " dut (\n";
            QStringList portConns;
            for (const auto &pin : inputPins) {
                portConns << QString("        .%1(%1)").arg(pin.m_varName);
            }
            for (const auto &pin : outputPins) {
                portConns << QString("        .%1(%1)").arg(pin.m_varName);
            }
            tb << portConns.join(",\n") << "\n";
            tb << "    );\n\n";

            tb << "    initial begin\n";
            tb << "        errors = 0;\n";

            // Initialize all inputs to 0 at time 0 to prevent x→1 being seen as
            // a spurious posedge in SystemVerilog (wiRedPanda inputs start at 0).
            tb << "        ";
            for (const auto &pin : inputPins) {
                tb << pin.m_varName << " = 0; ";
            }
            tb << "#10;\n\n";

            int checkCount = 0;
            for (int s = 0; s < seqSteps.size(); ++s) {
                const auto &step = seqSteps[s];
                const bool isCheck = !step.expectedOut.isEmpty();

                // Comment
                if (!step.comment.isEmpty()) {
                    tb << "        // " << step.comment << "\n";
                }

                // Set all inputs for this step
                tb << "        ";
                for (int i = 0; i < inputPins.size(); ++i) {
                    tb << inputPins[i].m_varName << " = " << step.inputBits[i] << "; ";
                }
                tb << "#10;\n";

                if (isCheck) {
                    // Check outputs — generic format for any number of outputs
                    QStringList checks;
                    for (int o = 0; o < outputPins.size(); ++o) {
                        checks << QString("%1 !== 1'b%2").arg(outputPins[o].m_varName).arg(step.expectedOut[o]);
                    }
                    tb << "        if (" << checks.join(" || ") << ") begin\n";

                    // Build generic display with all output names and values
                    QStringList fmtParts;
                    QStringList expectedArgs, gotArgs;
                    for (int o = 0; o < outputPins.size(); ++o) {
                        fmtParts << (outputPins[o].m_varName + "=%0b");
                        expectedArgs << QString::number(step.expectedOut[o]);
                        gotArgs << outputPins[o].m_varName;
                    }
                    tb << "            $display(\"FAIL step " << s << " ("
                       << step.comment << "): expected " << fmtParts.join(" ")
                       << " got " << fmtParts.join(" ") << "\",\n";
                    tb << "                " << expectedArgs.join(", ") << ", "
                       << gotArgs.join(", ") << ");\n";

                    tb << "            errors = errors + 1;\n";
                    tb << "        end\n";
                    ++checkCount;
                }
                tb << "\n";
            }

            tb << "        if (errors == 0)\n";
            tb << "            $display(\"PASS: %0d steps\", " << checkCount << ");\n";
            tb << "        else begin\n";
            tb << "            $display(\"FAIL: %0d/%0d steps failed\", errors, " << checkCount << ");\n";
            tb << "            $finish(1);\n";
            tb << "        end\n";
            tb << "        $finish;\n";
            tb << "    end\n";
            tb << "endmodule\n";
            tbFile.close();

            bool tbValid = validateWithTestbench(verilogPath, tbPath);
            if (!tbValid) {
                QFAIL(qPrintable(QString("Sequential testbench mismatch for \"%1\""
                    " (SystemVerilog output differs from wiRedPanda simulation)"
                    " - DUT: \"%2\" TB: \"%3\"").arg(icFile, verilogPath, tbPath)));
            } else {
                QFile::remove(tbPath);
            }
        }
    } else {
        // --- Combinational testbench: exhaustive or random vectors ---
        const int numInputs = static_cast<int>(switches.size());
        const int numOutputs = static_cast<int>(leds.size());

        int numPatterns;
        if (numInputs <= 12) {
            numPatterns = 1 << numInputs;
        } else {
            numPatterns = 258;
        }

        struct TestVector {
            int inputBits;
            QVector<int> outputBits; // per-output bit (supports >32 outputs)
        };
        QVector<TestVector> testVectors;
        testVectors.reserve(numPatterns);

        auto collectOutputBits = [&]() {
            QVector<int> bits(numOutputs);
            for (int o = 0; o < numOutputs; ++o) {
                bits[o] = TestUtils::getInputStatus(leds[o], 0) ? 1 : 0;
            }
            return bits;
        };

        if (numInputs <= 12) {
            for (int v = 0; v < numPatterns; ++v) {
                TestUtils::setMultiBitInput(switches, v);
                sim->update();
                testVectors.append({v, collectOutputBits()});
            }
        } else {
            auto collectVector = [&](int inputVal) {
                TestUtils::setMultiBitInput(switches, inputVal);
                sim->update();
                testVectors.append({inputVal, collectOutputBits()});
            };

            collectVector(0);
            collectVector((1 << numInputs) - 1);
            const int maxVal = 1 << numInputs;
            for (int i = 0; i < 256; ++i) {
                collectVector(QRandomGenerator::global()->bounded(maxVal));
            }
        }

        if (!testVectors.isEmpty()) {
            QString tbPath = debugDir.filePath(icInfo.baseName() + "_tb.sv");
            QFile tbFile(tbPath);
            if (tbFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream tb(&tbFile);
                tb << "`timescale 1ns/1ps\n";
                tb << "module testbench;\n";

                for (const auto &pin : inputPins) {
                    tb << "    reg " << pin.m_varName << ";\n";
                }
                for (const auto &pin : outputPins) {
                    tb << "    wire " << pin.m_varName << ";\n";
                }
                tb << "    integer errors;\n\n";

                tb << "    " << modName << " dut (\n";
                QStringList portConns;
                for (const auto &pin : inputPins) {
                    portConns << QString("        .%1(%1)").arg(pin.m_varName);
                }
                for (const auto &pin : outputPins) {
                    portConns << QString("        .%1(%1)").arg(pin.m_varName);
                }
                tb << portConns.join(",\n") << "\n";
                tb << "    );\n\n";

                tb << "    initial begin\n";
                tb << "        errors = 0;\n\n";

                for (int v = 0; v < testVectors.size(); ++v) {
                    const auto &tv = testVectors[v];

                    tb << "        ";
                    for (int i = 0; i < inputPins.size(); ++i) {
                        int bit = (tv.inputBits >> i) & 1;
                        tb << inputPins[i].m_varName << " = " << bit << "; ";
                    }
                    tb << "#10;\n";

                    QStringList checks;
                    for (int o = 0; o < outputPins.size(); ++o) {
                        int bit = tv.outputBits[o];
                        checks << QString("%1 !== 1'b%2").arg(outputPins[o].m_varName).arg(bit);
                    }

                    tb << "        if (" << checks.join(" || ") << ") begin\n";
                    tb << "            $display(\"FAIL vec " << v << ": in=%b"
                       << " out=%b\", {";

                    QStringList inputNames;
                    for (int i = static_cast<int>(inputPins.size()) - 1; i >= 0; --i) {
                        inputNames << inputPins[i].m_varName;
                    }
                    tb << inputNames.join(", ") << "}, {";

                    QStringList outputNames;
                    for (int o = static_cast<int>(outputPins.size()) - 1; o >= 0; --o) {
                        outputNames << outputPins[o].m_varName;
                    }
                    tb << outputNames.join(", ") << "});\n";
                    tb << "            errors = errors + 1;\n";
                    tb << "        end\n\n";
                }

                tb << "        if (errors == 0)\n";
                tb << "            $display(\"PASS: %0d vectors\", " << testVectors.size() << ");\n";
                tb << "        else begin\n";
                tb << "            $display(\"FAIL: %0d/%0d vectors failed\", errors, " << testVectors.size() << ");\n";
                tb << "            $finish(1);\n";
                tb << "        end\n";
                tb << "        $finish;\n";
                tb << "    end\n";
                tb << "endmodule\n";
                tbFile.close();

                bool tbValid = validateWithTestbench(verilogPath, tbPath);
                if (!tbValid) {
                    QFAIL(qPrintable(QString("Functional testbench mismatch for \"%1\""
                        " (SystemVerilog output differs from wiRedPanda simulation)"
                        " - DUT: \"%2\" TB: \"%3\"").arg(icFile, verilogPath, tbPath)));
                } else {
                    QFile::remove(tbPath);
                }
            }
        }
    }

    // Clean up generated SystemVerilog file on success (keep on mismatch for debugging)
    if (!QFile::exists(debugDir.filePath(icInfo.baseName() + "_tb.sv"))) {
        QFile::remove(verilogPath);
    }
}

// Level 2 - Combinational circuits

void TestSystemVerilogExport::testSystemVerilogExportHalfAdder()
{
    testSystemVerilogExportHelper("level2_half_adder.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportFullAdder1Bit()
{
    testSystemVerilogExportHelper("level2_full_adder_1bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportDecoder2to4()
{
    testSystemVerilogExportHelper("level2_decoder_2to4.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportMux2to1()
{
    testSystemVerilogExportHelper("level2_mux_2to1.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportDecoder3to8()
{
    testSystemVerilogExportHelper("level2_decoder_3to8.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportParityChecker()
{
    testSystemVerilogExportHelper("level2_parity_checker.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportDecoder4to16()
{
    testSystemVerilogExportHelper("level2_decoder_4to16.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportMux4to1()
{
    testSystemVerilogExportHelper("level2_mux_4to1.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportMux8to1()
{
    testSystemVerilogExportHelper("level2_mux_8to1.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportParityGenerator()
{
    testSystemVerilogExportHelper("level2_parity_generator.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportPriorityEncoder8to3()
{
    testSystemVerilogExportHelper("level2_priority_encoder_8to3.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportPriorityMux3to1()
{
    testSystemVerilogExportHelper("level2_priority_mux_3to1.panda");
}

// Level 1 - Sequential circuits

void TestSystemVerilogExport::testSystemVerilogExportDFlipFlop()
{
    testSystemVerilogExportHelper("level1_d_flip_flop.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportDLatch()
{
    testSystemVerilogExportHelper("level1_d_latch.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportJKFlipFlop()
{
    testSystemVerilogExportHelper("level1_jk_flip_flop.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportSRLatch()
{
    testSystemVerilogExportHelper("level1_sr_latch.panda");
}

// Level 3 - Medium combinational

void TestSystemVerilogExport::testSystemVerilogExportAluSelector5Way()
{
    testSystemVerilogExportHelper("level3_alu_selector_5way.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportBcd7SegmentDecoder()
{
    testSystemVerilogExportHelper("level3_bcd_7segment_decoder.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportComparator4Bit()
{
    testSystemVerilogExportHelper("level3_comparator_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportComparator4BitEquality()
{
    testSystemVerilogExportHelper("level3_comparator_4bit_equality.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRegister1Bit()
{
    testSystemVerilogExportHelper("level3_register_1bit.panda");
}

// Level 4 - 4-bit datapaths

void TestSystemVerilogExport::testSystemVerilogExportRippleAdder4Bit()
{
    // Note: Uses level 2 full adders which are combinational - should work
    testSystemVerilogExportHelper("level4_ripple_adder_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRegister4Bit()
{
    testSystemVerilogExportHelper("level4_register_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportBinaryCounter4Bit()
{
    testSystemVerilogExportHelper("level4_binary_counter_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportBusMux4Bit()
{
    testSystemVerilogExportHelper("level4_bus_mux_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportBusMux8Bit()
{
    testSystemVerilogExportHelper("level4_bus_mux_8bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportComparator4BitLevel4()
{
    testSystemVerilogExportHelper("level4_comparator_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportJohnsonCounter4Bit()
{
    testSystemVerilogExportHelper("level4_johnson_counter_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRam4x1()
{
    testSystemVerilogExportHelper("level4_ram_4x1.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRam8x1()
{
    testSystemVerilogExportHelper("level4_ram_8x1.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRingCounter4Bit()
{
    testSystemVerilogExportHelper("level4_ring_counter_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRippleAlu4Bit()
{
    testSystemVerilogExportHelper("level4_ripple_alu_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportShiftRegisterPiso()
{
    testSystemVerilogExportHelper("level4_shift_register_piso.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportShiftRegisterSipo()
{
    testSystemVerilogExportHelper("level4_shift_register_sipo.panda");
}

// Level 5 - Advanced 4-bit

void TestSystemVerilogExport::testSystemVerilogExportBarrelRotator()
{
    testSystemVerilogExportHelper("level5_barrel_rotator.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportBarrelShifter4Bit()
{
    testSystemVerilogExportHelper("level5_barrel_shifter_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportClockGatedDecoder()
{
    testSystemVerilogExportHelper("level5_clock_gated_decoder.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportController4Bit()
{
    testSystemVerilogExportHelper("level5_controller_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportInstructionDecoder4Bit()
{
    testSystemVerilogExportHelper("level5_instruction_decoder_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportLoadableCounter4Bit()
{
    testSystemVerilogExportHelper("level5_loadable_counter_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportModuloCounter4Bit()
{
    testSystemVerilogExportHelper("level5_modulo_counter_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportProgramCounter4Bit()
{
    testSystemVerilogExportHelper("level5_program_counter_4bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRegisterFile4x4()
{
    testSystemVerilogExportHelper("level5_register_file_4x4.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRegisterFile8x8Level5()
{
    testSystemVerilogExportHelper("level5_register_file_8x8.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportStackMemoryInterface()
{
    testSystemVerilogExportHelper("level6_stack_memory_interface.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportUpDownCounter4Bit()
{
    testSystemVerilogExportHelper("level5_up_down_counter_4bit.panda");
}

// Level 6 - 8-bit subsystems

void TestSystemVerilogExport::testSystemVerilogExportRippleAdder8Bit()
{
    testSystemVerilogExportHelper("level6_ripple_adder_8bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportALU8Bit()
{
    testSystemVerilogExportHelper("level6_alu_8bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportProgramCounter8BitArithmetic()
{
    testSystemVerilogExportHelper("level6_program_counter_8bit_arithmetic.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRam256x8()
{
    testSystemVerilogExportHelper("level6_ram_256x8.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRegister8Bit()
{
    testSystemVerilogExportHelper("level6_register_8bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportRegisterFile8x8Level6()
{
    testSystemVerilogExportHelper("level6_register_file_8x8.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportStackPointer8Bit()
{
    testSystemVerilogExportHelper("level6_stack_pointer_8bit.panda");
}

// Level 7 - 16-bit and CPU subsystems

void TestSystemVerilogExport::testSystemVerilogExportAlu16Bit()
{
    testSystemVerilogExportHelper("level7_alu_16bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportCpuProgramCounter8Bit()
{
    testSystemVerilogExportHelper("level7_cpu_program_counter_8bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportDataForwardingUnit()
{
    testSystemVerilogExportHelper("level7_data_forwarding_unit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportExecutionDatapath()
{
    testSystemVerilogExportHelper("level7_execution_datapath.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportFlagRegister()
{
    testSystemVerilogExportHelper("level7_flag_register.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportInstructionDecoder8Bit()
{
    testSystemVerilogExportHelper("level7_instruction_decoder_8bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportInstructionMemoryInterface()
{
    testSystemVerilogExportHelper("level7_instruction_memory_interface.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportInstructionRegister8Bit()
{
    testSystemVerilogExportHelper("level7_instruction_register_8bit.panda");
}

// Level 8 - CPU pipeline stages

void TestSystemVerilogExport::testSystemVerilogExportDecodeStage()
{
    testSystemVerilogExportHelper("level8_decode_stage.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportExecuteStage()
{
    testSystemVerilogExportHelper("level8_execute_stage.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportFetchStage()
{
    testSystemVerilogExportHelper("level8_fetch_stage.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportMemoryStage()
{
    testSystemVerilogExportHelper("level8_memory_stage.panda");
}

// Level 9 - Full CPUs

void TestSystemVerilogExport::testSystemVerilogExportCpu16BitRisc()
{
    testSystemVerilogExportHelper("level9_cpu_16bit_risc.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportFetchStage16Bit()
{
    testSystemVerilogExportHelper("level9_fetch_stage_16bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportMultiCycleCpu8Bit()
{
    QSKIP("Gated-clock timing mismatch: wiRedPanda's topological-sort engine evaluates "
          "counter DFFs before gated clocks (using NEW state), while SystemVerilog's "
          "event-driven model fires gated clocks with OLD state (non-blocking delta-cycle "
          "semantics). Will be resolved when the engine transitions to event-driven.");
}

void TestSystemVerilogExport::testSystemVerilogExportSingleCycleCpu8Bit()
{
    testSystemVerilogExportHelper("level9_single_cycle_cpu_8bit.panda");
}

