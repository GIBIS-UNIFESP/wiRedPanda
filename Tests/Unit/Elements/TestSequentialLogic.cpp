// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestSequentialLogic.h"

#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "Tests/Common/TestUtils.h"

void TestSequentialLogic::testDFlipFlop()
{
    DFlipFlop elm;
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 4);
    QCOMPARE(elm.maxInputSize(), 4);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::DFlipFlop);
}

void TestSequentialLogic::testDLatch()
{
    DLatch elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.maxInputSize(), 2);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::DLatch);
}

void TestSequentialLogic::testJKFlipFlop()
{
    JKFlipFlop elm;
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 5);
    QCOMPARE(elm.maxInputSize(), 5);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::JKFlipFlop);
}

void TestSequentialLogic::testSRFlipFlop()
{
    SRFlipFlop elm;
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 5);
    QCOMPARE(elm.maxInputSize(), 5);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::SRFlipFlop);
}

void TestSequentialLogic::testSRLatch()
{
    SRLatch elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.maxInputSize(), 2);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::SRLatch);
}

void TestSequentialLogic::testTFlipFlop()
{
    TFlipFlop elm;
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 4);
    QCOMPARE(elm.maxInputSize(), 4);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::TFlipFlop);
}

// ============================================================================
// Behavioral Tests
// ============================================================================

void TestSequentialLogic::testDFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swD, swClk;
    DFlipFlop dff;
    Led ledQ, ledNQ;

    builder.add(&swD, &swClk, &dff, &ledQ, &ledNQ);
    builder.connect(&swD, 0, &dff, 0);    // D
    builder.connect(&swClk, 0, &dff, 1);  // Clock
    builder.connect(&dff, 0, &ledQ, 0);   // Q
    builder.connect(&dff, 1, &ledNQ, 0);  // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // D=1: capture into m_lastValue, then rising edge → Q=1
    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // Hold: D→0, no clock → Q stays 1
    swD.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // Clock with D=0 → Q=0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);
}

void TestSequentialLogic::testDLatchBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swD, swEnable;
    DLatch dlatch;
    Led ledQ, ledNQ;

    builder.add(&swD, &swEnable, &dlatch, &ledQ, &ledNQ);
    builder.connect(&swD, 0, &dlatch, 0);      // D
    builder.connect(&swEnable, 0, &dlatch, 1); // Enable
    builder.connect(&dlatch, 0, &ledQ, 0);     // Q
    builder.connect(&dlatch, 1, &ledNQ, 0);    // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // Enable=1, D=1: transparent → Q=1
    swEnable.setOn(true);
    swD.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // Enable=0, D→0: latched → Q stays 1
    swEnable.setOn(false);
    swD.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // Enable=1, D=0: transparent again → Q=0
    swEnable.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);
}

void TestSequentialLogic::testJKFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swJ, swClk, swK;
    JKFlipFlop jkff;
    Led ledQ, ledNQ;

    builder.add(&swJ, &swClk, &swK, &jkff, &ledQ, &ledNQ);
    builder.connect(&swJ, 0, &jkff, 0);   // J
    builder.connect(&swClk, 0, &jkff, 1); // Clock
    builder.connect(&swK, 0, &jkff, 2);   // K
    builder.connect(&jkff, 0, &ledQ, 0);  // Q
    builder.connect(&jkff, 1, &ledNQ, 0); // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // J=1, K=0: Set → Q=1
    swJ.setOn(true);
    swK.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // J=0, K=1: Reset → Q=0
    swJ.setOn(false);
    swK.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // J=1, K=1: Toggle → Q=1
    swJ.setOn(true);
    swK.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // J=1, K=1: Toggle again → Q=0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);
}

void TestSequentialLogic::testSRFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swS, swClk, swR;
    SRFlipFlop srff;
    Led ledQ, ledNQ;

    builder.add(&swS, &swClk, &swR, &srff, &ledQ, &ledNQ);
    builder.connect(&swS, 0, &srff, 0);   // S
    builder.connect(&swClk, 0, &srff, 1); // Clock
    builder.connect(&swR, 0, &srff, 2);   // R
    builder.connect(&srff, 0, &ledQ, 0);  // Q
    builder.connect(&srff, 1, &ledNQ, 0); // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // S=1, R=0: Set → Q=1
    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // S=0, R=0: Hold → Q stays 1
    swS.setOn(false);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // S=0, R=1: Reset → Q=0
    swR.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);
}

void TestSequentialLogic::testSRLatchBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swS, swR;
    SRLatch srlatch;
    Led ledQ, ledNQ;

    builder.add(&swS, &swR, &srlatch, &ledQ, &ledNQ);
    builder.connect(&swS, 0, &srlatch, 0); // Set
    builder.connect(&swR, 0, &srlatch, 1); // Reset
    builder.connect(&srlatch, 0, &ledQ, 0);  // Q
    builder.connect(&srlatch, 1, &ledNQ, 0); // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // S=1, R=0: Set → Q=1
    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // S=0, R=0: Hold → Q stays 1
    swS.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // S=0, R=1: Reset → Q=0
    swR.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // S=1, R=1: Invalid state → Q=0, ~Q=0
    swS.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);
}

void TestSequentialLogic::testTFlipFlopBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swT, swClk;
    TFlipFlop tff;
    Led ledQ, ledNQ;

    builder.add(&swT, &swClk, &tff, &ledQ, &ledNQ);
    builder.connect(&swT, 0, &tff, 0);    // T
    builder.connect(&swClk, 0, &tff, 1);  // Clock
    builder.connect(&tff, 0, &ledQ, 0);   // Q
    builder.connect(&tff, 1, &ledNQ, 0);  // ~Q

    auto *sim = builder.initSimulation();
    sim->update();

    // Initial state: Q=0, ~Q=1
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // T=0: clock pulse → no toggle (hold)
    swT.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);

    // T=1: clock pulse → toggle Q=0→1
    swT.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), true);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), false);

    // T=1: clock pulse → toggle Q=1→0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::getInputStatus(&ledQ), false);
    QCOMPARE(TestUtils::getInputStatus(&ledNQ), true);
}

