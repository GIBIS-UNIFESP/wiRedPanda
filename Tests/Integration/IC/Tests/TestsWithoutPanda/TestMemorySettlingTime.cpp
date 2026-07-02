// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestMemorySettlingTime.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "Tests/Common/TestUtils.h"

using TestUtils::inputStatus;

// NOTE: these were originally "settling time" benchmarks, but the simulator runs
// in functional (zero-delay) mode where every combinational circuit settles in a
// single update() regardless of logic depth. There is therefore no multi-cycle
// settling to measure, and the old timing comparisons (cascaded >= single-stage,
// settlingTime in [1,100], etc.) were all tautologies. The tests now verify what
// these decoders can actually be checked for: correct one-hot decoding.

// NOTE: these were originally "settling time" benchmarks, but the simulator runs
// in functional (zero-delay) mode where every combinational circuit settles in a
// single update() regardless of logic depth. There is therefore no multi-cycle
// settling to measure, and the old timing comparisons (cascaded >= single-stage,
// settlingTime in [1,100], etc.) were all tautologies. The tests now verify what
// these decoders can actually be checked for: correct one-hot decoding.

// ============================================================
// Helper: Single-Stage 6-bit Decoder
// ============================================================

/**
 * @brief Build a single-stage 6-bit address decoder (64 outputs).
 *
 * Direct decoding: each output is a 7-input AND of the six (matched/inverted)
 * address bits plus the control signal.
 */
struct SingleStageDecoderCircuit {
    InputSwitch addressBits[6];
    InputSwitch controlSignal;
    Led outputs[64];
    And decoders[64];
    Not notGates[6];
};

void buildSingleStageDecoder6bit(WorkSpace* workspace,
                                 InputSwitch*& address,
                                 InputSwitch*& control,
                                 Led*& outputs,
                                 std::unique_ptr<SingleStageDecoderCircuit>& circuit,
                                 Simulation*& sim)
{
    CircuitBuilder builder(workspace->scene());

    circuit = std::make_unique<SingleStageDecoderCircuit>();

    address = circuit->addressBits;
    control = &circuit->controlSignal;
    outputs = circuit->outputs;

    for (int i = 0; i < 6; i++) {
        builder.add(&circuit->addressBits[i], &circuit->notGates[i]);
    }
    builder.add(&circuit->controlSignal);
    for (int i = 0; i < 64; i++) {
        circuit->decoders[i].setInputSize(7);  // 6 address bits + control
        builder.add(&circuit->decoders[i], &circuit->outputs[i]);
    }

    for (int i = 0; i < 6; i++) {
        builder.connect(&circuit->addressBits[i], 0, &circuit->notGates[i], 0);
    }

    for (int addr = 0; addr < 64; addr++) {
        And *decoder = &circuit->decoders[addr];

        for (int bit = 0; bit < 6; bit++) {
            const bool bitSet = (addr & (1 << bit)) != 0;
            GraphicElement *src = bitSet ? static_cast<GraphicElement *>(&circuit->addressBits[bit])
                                         : static_cast<GraphicElement *>(&circuit->notGates[bit]);
            builder.connect(src, 0, decoder, bit);
        }
        builder.connect(&circuit->controlSignal, 0, decoder, 6);
        builder.connect(decoder, 0, &circuit->outputs[addr], 0);
    }

    sim = builder.initSimulation();
}

// ============================================================
// Helper: Cascaded 6-bit Decoder (2-level)
// ============================================================

/**
 * @brief Build a cascaded 6-bit address decoder (2-level tree).
 *
 * Level 1 decodes the low three address bits; level 2 ANDs that with the high
 * three (matched/inverted) bits and the control signal. This is a genuine 6-bit
 * decode — the previous version wired only address bits 0 and 1, so it was really
 * a 2-bit decoder whose outputs were not one-hot.
 */
struct CascadedDecoderCircuit {
    InputSwitch addressBits[6];
    InputSwitch controlSignal;
    Led outputs[64];
    And level1Decoders[64];  // low 3 address bits
    And level2Decoders[64];  // level1 + high 3 bits + control
    Not notGates[6];
};

void buildCascadedDecoder6bit(WorkSpace* workspace,
                              InputSwitch*& address,
                              InputSwitch*& control,
                              Led*& outputs,
                              std::unique_ptr<CascadedDecoderCircuit>& circuit,
                              Simulation*& sim)
{
    CircuitBuilder builder(workspace->scene());

    circuit = std::make_unique<CascadedDecoderCircuit>();

    address = circuit->addressBits;
    control = &circuit->controlSignal;
    outputs = circuit->outputs;

    for (int i = 0; i < 6; i++) {
        builder.add(&circuit->addressBits[i], &circuit->notGates[i]);
    }
    builder.add(&circuit->controlSignal);
    for (int i = 0; i < 64; i++) {
        circuit->level1Decoders[i].setInputSize(3);  // low 3 address bits
        circuit->level2Decoders[i].setInputSize(5);  // level1 + high 3 bits + control
        builder.add(&circuit->level1Decoders[i], &circuit->level2Decoders[i], &circuit->outputs[i]);
    }

    for (int i = 0; i < 6; i++) {
        builder.connect(&circuit->addressBits[i], 0, &circuit->notGates[i], 0);
    }

    for (int addr = 0; addr < 64; addr++) {
        And *l1 = &circuit->level1Decoders[addr];
        And *l2 = &circuit->level2Decoders[addr];

        // Level 1: decode the low three address bits.
        for (int bit = 0; bit < 3; bit++) {
            const bool bitSet = (addr & (1 << bit)) != 0;
            GraphicElement *src = bitSet ? static_cast<GraphicElement *>(&circuit->addressBits[bit])
                                         : static_cast<GraphicElement *>(&circuit->notGates[bit]);
            builder.connect(src, 0, l1, bit);
        }

        // Level 2: AND(level1, high three bits, control).
        builder.connect(l1, 0, l2, 0);
        for (int bit = 3; bit < 6; bit++) {
            const bool bitSet = (addr & (1 << bit)) != 0;
            GraphicElement *src = bitSet ? static_cast<GraphicElement *>(&circuit->addressBits[bit])
                                         : static_cast<GraphicElement *>(&circuit->notGates[bit]);
            builder.connect(src, 0, l2, bit - 2);
        }
        builder.connect(&circuit->controlSignal, 0, l2, 4);

        builder.connect(l2, 0, &circuit->outputs[addr], 0);
    }

    sim = builder.initSimulation();
}

// ============================================================
// Helper: one-hot check
// ============================================================

/// Returns true iff exactly outputs[activeAddr] is high among the 64 outputs.
static bool isOneHot(Led *outputs, int activeAddr)
{
    for (int i = 0; i < 64; ++i) {
        if (inputStatus(&outputs[i]) != (i == activeAddr)) {
            return false;
        }
    }
    return true;
}

// ============================================================
// Test Implementation
// ============================================================

void TestMemorySettlingTime::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestMemorySettlingTime::testDecoderSettlingTime_singlestage_data()
{
    QTest::addColumn<int>("addressValue");

    QTest::newRow("addr_0x00") << 0x00;
    QTest::newRow("addr_0x01") << 0x01;
    QTest::newRow("addr_0x15") << 0x15;
    QTest::newRow("addr_0x2A") << 0x2A;
    QTest::newRow("addr_0x3F") << 0x3F;
}

void TestMemorySettlingTime::testDecoderSettlingTime_singlestage()
{
    QFETCH(int, addressValue);

    InputSwitch *address, *control;
    Led *outputs;

    auto workspace = std::make_unique<WorkSpace>();
    std::unique_ptr<SingleStageDecoderCircuit> circuit;
    Simulation *sim = nullptr;
    buildSingleStageDecoder6bit(workspace.get(), address, control, outputs, circuit, sim);

    sim->update();
    for (int bit = 0; bit < 6; bit++) {
        address[bit].setOn((addressValue >> bit) & 1);
    }
    control->setOn(true);
    sim->update();

    // Exactly the addressed output is active; all 63 others are inactive.
    QVERIFY2(isOneHot(outputs, addressValue),
             qPrintable(QString("single-stage decode of 0x%1 is not one-hot").arg(addressValue, 0, 16)));
}

void TestMemorySettlingTime::testDecoderSettlingTime_cascaded_data()
{
    QTest::addColumn<int>("addressValue");

    QTest::newRow("addr_0x00") << 0x00;
    QTest::newRow("addr_0x01") << 0x01;
    QTest::newRow("addr_0x15") << 0x15;
    QTest::newRow("addr_0x2A") << 0x2A;
    QTest::newRow("addr_0x3F") << 0x3F;
}

void TestMemorySettlingTime::testDecoderSettlingTime_cascaded()
{
    QFETCH(int, addressValue);

    InputSwitch *address, *control;
    Led *outputs;

    auto workspace = std::make_unique<WorkSpace>();
    std::unique_ptr<CascadedDecoderCircuit> circuit;
    Simulation *sim = nullptr;
    buildCascadedDecoder6bit(workspace.get(), address, control, outputs, circuit, sim);

    sim->update();
    for (int bit = 0; bit < 6; bit++) {
        address[bit].setOn((addressValue >> bit) & 1);
    }
    control->setOn(true);
    sim->update();

    // The 2-level cascade is a genuine 6-bit decoder: one-hot like the single
    // stage. (Previously it wired only 2 address bits, so this was never checked.)
    QVERIFY2(isOneHot(outputs, addressValue),
             qPrintable(QString("cascaded decode of 0x%1 is not one-hot").arg(addressValue, 0, 16)));
}

void TestMemorySettlingTime::testReadMuxSettlingTime_data()
{
    QTest::addColumn<int>("addressValue");

    QTest::newRow("addr_0x2A") << 0x2A;
    QTest::newRow("addr_0x15") << 0x15;
    QTest::newRow("addr_0x00") << 0x00;
    QTest::newRow("addr_0x3F") << 0x3F;
}

void TestMemorySettlingTime::testReadMuxSettlingTime()
{
    QFETCH(int, addressValue);

    // Both decoder topologies must produce the same one-hot result for a given
    // address. (The old test compared cascaded vs single-stage "settling cycle
    // counts", which are identical in zero-delay mode and so proved nothing.)
    InputSwitch *addr_ss, *ctrl_ss, *addr_cas, *ctrl_cas;
    Led *out_ss, *out_cas;

    auto workspace_ss = std::make_unique<WorkSpace>();
    std::unique_ptr<SingleStageDecoderCircuit> circuit_ss;
    Simulation *sim_ss = nullptr;
    buildSingleStageDecoder6bit(workspace_ss.get(), addr_ss, ctrl_ss, out_ss, circuit_ss, sim_ss);

    auto workspace_cas = std::make_unique<WorkSpace>();
    std::unique_ptr<CascadedDecoderCircuit> circuit_cas;
    Simulation *sim_cas = nullptr;
    buildCascadedDecoder6bit(workspace_cas.get(), addr_cas, ctrl_cas, out_cas, circuit_cas, sim_cas);

    sim_ss->update();
    sim_cas->update();
    for (int bit = 0; bit < 6; bit++) {
        addr_ss[bit].setOn((addressValue >> bit) & 1);
        addr_cas[bit].setOn((addressValue >> bit) & 1);
    }
    ctrl_ss->setOn(true);
    ctrl_cas->setOn(true);
    sim_ss->update();
    sim_cas->update();

    QVERIFY2(isOneHot(out_ss, addressValue), "single-stage decode not one-hot");
    QVERIFY2(isOneHot(out_cas, addressValue), "cascaded decode not one-hot");
}

void TestMemorySettlingTime::testSettlingTimeBenchmarkSummary()
{
    // Verify both decoders decode every tested address correctly (one-hot). The
    // old "average settling time" comparison was tautological in zero-delay mode.
    InputSwitch *addr_ss, *ctrl_ss, *addr_cas, *ctrl_cas;
    Led *out_ss, *out_cas;

    auto workspace_ss = std::make_unique<WorkSpace>();
    std::unique_ptr<SingleStageDecoderCircuit> circuit_ss;
    Simulation *sim_ss = nullptr;
    buildSingleStageDecoder6bit(workspace_ss.get(), addr_ss, ctrl_ss, out_ss, circuit_ss, sim_ss);

    auto workspace_cas = std::make_unique<WorkSpace>();
    std::unique_ptr<CascadedDecoderCircuit> circuit_cas;
    Simulation *sim_cas = nullptr;
    buildCascadedDecoder6bit(workspace_cas.get(), addr_cas, ctrl_cas, out_cas, circuit_cas, sim_cas);

    const int testAddresses[] = {0x00, 0x01, 0x15, 0x2A, 0x3F, 0x20, 0x10, 0x0F};

    sim_ss->update();
    sim_cas->update();
    ctrl_ss->setOn(true);
    ctrl_cas->setOn(true);

    for (int addr : testAddresses) {
        for (int bit = 0; bit < 6; bit++) {
            addr_ss[bit].setOn((addr >> bit) & 1);
            addr_cas[bit].setOn((addr >> bit) & 1);
        }
        sim_ss->update();
        sim_cas->update();

        QVERIFY2(isOneHot(out_ss, addr),
                 qPrintable(QString("single-stage 0x%1 not one-hot").arg(addr, 0, 16)));
        QVERIFY2(isOneHot(out_cas, addr),
                 qPrintable(QString("cascaded 0x%1 not one-hot").arg(addr, 0, 16)));
    }
}
