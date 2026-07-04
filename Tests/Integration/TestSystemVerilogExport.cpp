// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestSystemVerilogExport.h"

#include <memory>

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
#include "App/Wiring/Connection.h"
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
        } else if (name == "reset" || name == "sp_reset" || name == "init") {
            // "init" is an active-HIGH seed input (e.g. Johnson/ring counters
            // load a known non-zero state). Treating it like a reset lets the
            // harness assert it to establish a defined power-on state, so the
            // engine and the exported SystemVerilog agree on the first sample
            // instead of diverging on the master-slave DFF's ambiguous cold
            // start.
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

// Seeded-random differential stimulus parameters (deterministic across runs).
static constexpr quint32 kDiffSeed = 0x5EEDC0DEU;
static constexpr int kDiffSteps = 48;

void TestSystemVerilogExport::initTestCase()
{
#ifndef Q_OS_LINUX
    QSKIP("SystemVerilog export tests require iverilog/yosys/verilator (Linux only)");
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
    auto sw = std::make_unique<InputSwitch>();
    auto txNode = std::unique_ptr<Node>(qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node)));
    QVERIFY(txNode);
    txNode->setLabel("CLK");
    txNode->setWirelessMode(WirelessMode::Tx);

    auto rxNode = std::unique_ptr<Node>(qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node)));
    QVERIFY(rxNode);
    rxNode->setLabel("CLK");
    rxNode->setWirelessMode(WirelessMode::Rx);

    auto led = std::make_unique<Led>();

    // Physical wire: InputSwitch → Tx node, Rx node → LED
    auto *swOut = sw->outputPort(0);
    auto *txIn = txNode->inputPort(0);
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(swOut);
    conn1->setEndPort(txIn);

    auto *rxOut = rxNode->outputPort(0);
    auto *ledIn = led->inputPort(0);
    auto conn2 = std::make_unique<Connection>();
    conn2->setStartPort(rxOut);
    conn2->setEndPort(ledIn);

    QVector<GraphicElement *> elements{sw.get(), txNode.get(), rxNode.get(), led.get()};

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString verilogPath = tempDir.filePath("test_wireless.sv");

    SystemVerilogCodeGen generator(verilogPath, elements);
    try {
        generator.generate();
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("SystemVerilog generation threw an exception for wireless circuit: %1").arg(e.what())));
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
}

void TestSystemVerilogExport::testWirelessOrphanedRxCodegen()
{
    // An Rx node with no matching Tx must not crash SystemVerilog code generation.
    auto rxNode = std::unique_ptr<Node>(qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node)));
    QVERIFY(rxNode);
    rxNode->setLabel("MISSING_TX");
    rxNode->setWirelessMode(WirelessMode::Rx);

    auto led = std::make_unique<Led>();

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(rxNode->outputPort(0));
    conn->setEndPort(led->inputPort(0));

    QVector<GraphicElement *> elements{rxNode.get(), led.get()};

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString verilogPath = tempDir.filePath("test_orphan_rx.sv");

    SystemVerilogCodeGen generator(verilogPath, elements);
    try {
        generator.generate();
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("SystemVerilog generation threw an exception for orphaned Rx circuit: %1").arg(e.what())));
    }

    QFile file(verilogPath);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             "Generated SystemVerilog file should be readable");
    const QString content = file.readAll();
    QVERIFY2(!content.isEmpty(), "Generated SystemVerilog should not be empty");
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

    // Verify file was created and read generated content
    QVERIFY2(QFileInfo::exists(verilogPath), "SystemVerilog file should be created");

    QFile generatedFile(verilogPath);
    QVERIFY2(generatedFile.open(QIODevice::ReadOnly | QIODevice::Text),
        qPrintable(QString("Could not read generated file: %1").arg(verilogPath)));
    const QString content = QString::fromUtf8(generatedFile.readAll());
    generatedFile.close();
    QVERIFY2(!content.isEmpty(), "Generated SystemVerilog should not be empty");

    // Generate or compare against expected SystemVerilog output
    const bool generateMode = qEnvironmentVariableIsSet("GENERATE_EXPECTED_SYSTEMVERILOG");
    const QString expectedDir = TestUtils::systemVerilogExpectedDir();
    const QString baseName = icFile.split('/').last().replace(".panda", "");
    const QString expectedPath = expectedDir + baseName + ".sv";

    if (generateMode) {
        // Full validation with external tools
        QVERIFY2(validateWithIverilog(verilogPath), qPrintable(QString("iverilog validation failed for %1").arg(icFile)));
        QVERIFY2(validateWithYosys(verilogPath), qPrintable(QString("yosys validation failed for %1").arg(icFile)));
        QVERIFY2(validateWithVerilator(verilogPath), qPrintable(QString("verilator validation failed for %1").arg(icFile)));

        const auto &inputPins = generator.inputMap();
        const auto &outputPins = generator.outputMap();
        const QString &modName = generator.moduleName();

        // Sequential stimulus: seeded-random differential vectors driven through
        // the engine oracle (replaces the old hand-written per-type tables). The
        // clock (if any) is pulsed with a mid-pulse sample, which makes the check
        // edge-polarity sensitive. A circuit is sequential if it is a recognised
        // latch or exposes a clock.
        QVector<TestUtils::DiffStep> seqSteps;
        // Active-low async controls (preset/clear) held inactive throughout the
        // differential run: two of them low is the forbidden state, whose
        // metastable release is undefined and resolves differently in the engine
        // vs a faithful gate-level export — so the check stays out of it. The
        // testbench's t=0 init must hold these HIGH too, else SV starts inside
        // the forbidden state while the engine never does.
        QVector<int> holdInactive;
        QVector<int> asyncHighReset;
        {
            const PortRoles roles = detectPortRoles(ic);
            const bool isSequential = (seqType != SequentialICType::None) || (roles.clockIdx >= 0);
            if (isSequential) {
                for (int i = 0; i < ic->inputSize(); ++i) {
                    const QString nm = ic->inputPort(i)->name().toLower().trimmed();
                    if (nm == "preset" || nm == "clear") {
                        holdInactive.append(i);
                    } else if (nm == "reset" || nm == "sp_reset" || nm == "init") {
                        // Active-high boundary reset/seed: the preamble pulses it
                        // to put the embedded flip-flops in a defined state both
                        // the engine and the export agree on (else the run starts
                        // from the flip-flops' ambiguous power-on value).
                        asyncHighReset.append(i);
                    }
                }
                seqSteps = TestUtils::generateDifferentialVectors(
                    switches, leds, sim, roles.clockIdx, holdInactive, asyncHighReset, kDiffSeed, kDiffSteps);
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

                // Initialize inputs at t=0 to prevent x→1 being seen as a spurious
                // posedge. Held async controls (preset/clear) start HIGH (inactive)
                // to match the engine — otherwise SV would power on in the
                // forbidden state and resolve differently from the oracle.
                tb << "        ";
                for (int i = 0; i < inputPins.size(); ++i) {
                    tb << inputPins[i].m_varName << " = " << (holdInactive.contains(i) ? 1 : 0) << "; ";
                }
                tb << "#10;\n\n";

                int checkCount = 0;
                for (int s = 0; s < seqSteps.size(); ++s) {
                    const auto &step = seqSteps[s];
                    const bool isCheck = !step.expectedOut.isEmpty();

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
                        tb << "            $display(\"FAIL step " << s
                           << ": expected " << fmtParts.join(" ")
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
                }
                QFile::remove(tbPath);
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

            // Inputs are stored one bit per switch (not packed into an int):
            // wide ICs (16-bit ALU = 35 inputs, data forwarding unit = 34)
            // exceed the 31 addressable bits of a packed int, so a packed
            // representation silently truncates the vector space. Per-switch
            // bits have no width limit and give uniform coverage on every lane.
            struct TestVector {
                QVector<int> inputBits;  // per-input bit
                QVector<int> outputBits; // per-output bit (supports >32 outputs)
            };
            QVector<TestVector> testVectors;
            testVectors.reserve(numPatterns);

            auto collectOutputBits = [&]() {
                QVector<int> bits(numOutputs);
                for (int o = 0; o < numOutputs; ++o) {
                    bits[o] = TestUtils::inputStatus(leds[o], 0) ? 1 : 0;
                }
                return bits;
            };

            auto collectVector = [&](const QVector<int> &bits) {
                for (int i = 0; i < numInputs; ++i) {
                    switches[i]->setOn(bits.at(i) != 0);
                }
                sim->update();
                testVectors.append({bits, collectOutputBits()});
            };

            if (numInputs <= 12) {
                for (int v = 0; v < numPatterns; ++v) {
                    QVector<int> bits(numInputs);
                    for (int i = 0; i < numInputs; ++i) {
                        bits[i] = (v >> i) & 1;
                    }
                    collectVector(bits);
                }
            } else {
                // Seeded so regeneration is reproducible (the sequential path
                // uses the same seed for its differential vectors).
                QRandomGenerator rng(kDiffSeed);

                collectVector(QVector<int>(numInputs, 0));  // all-zeros
                collectVector(QVector<int>(numInputs, 1));  // all-ones
                for (int i = 0; i < 256; ++i) {
                    QVector<int> bits(numInputs);
                    for (int b = 0; b < numInputs; ++b) {
                        bits[b] = rng.bounded(2);
                    }
                    collectVector(bits);
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
                            const int bit = tv.inputBits.value(i, 0);
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
                    }
                    QFile::remove(tbPath);
                }
            }
        }

        // Save validated code as reference
        QDir().mkpath(expectedDir);
        QFile expectedFile(expectedPath);
        QVERIFY2(expectedFile.open(QIODevice::WriteOnly | QIODevice::Text),
            qPrintable(QString("Could not write expected file: %1").arg(expectedPath)));
        expectedFile.write(content.toUtf8());
        expectedFile.close();
        qInfo() << "Generated expected SystemVerilog file:" << expectedPath;
    } else {
        // Compare against expected output (fast, no external tools)
        if (QFileInfo::exists(expectedPath)) {
            QFile expectedFile(expectedPath);
            QVERIFY2(expectedFile.open(QIODevice::ReadOnly | QIODevice::Text),
                qPrintable(QString("Could not read expected file: %1").arg(expectedPath)));
            const QString expectedContent = QString::fromUtf8(expectedFile.readAll());
            expectedFile.close();
            QCOMPARE(content, expectedContent);
        } else {
            QFAIL(qPrintable(QString("Reference file missing: %1 — run with GENERATE_EXPECTED_SYSTEMVERILOG=1 to create it").arg(expectedPath)));
        }
    }

    QFile::remove(verilogPath);
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

void TestSystemVerilogExport::testSystemVerilogExportRam8x8()
{
    testSystemVerilogExportHelper("level6_ram_8x8.panda");
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
    testSystemVerilogExportHelper("level9_multi_cycle_cpu_8bit.panda");
}

void TestSystemVerilogExport::testSystemVerilogExportSingleCycleCpu8Bit()
{
    testSystemVerilogExportHelper("level9_single_cycle_cpu_8bit.panda");
}
