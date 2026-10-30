// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestSequentialLogic.h"

#include <functional>
#include <memory>

#include <QDataStream>
#include <QImage>
#include <QPainter>
#include <QScopeGuard>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryDir>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
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
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // D=1: capture into m_lastValue, then rising edge → Q=1
    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Hold: D→0, no clock → Q stays 1
    swD.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Clock with D=0 → Q=0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
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
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // Enable=1, D=1: transparent → Q=1
    swEnable.setOn(true);
    swD.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Enable=0, D→0: latched → Q stays 1
    swEnable.setOn(false);
    swD.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // Enable=1, D=0: transparent again → Q=0
    swEnable.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
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
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // J=1, K=0: Set → Q=1
    swJ.setOn(true);
    swK.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // J=0, K=1: Reset → Q=0
    swJ.setOn(false);
    swK.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // J=1, K=1: Toggle → Q=1
    swJ.setOn(true);
    swK.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // J=1, K=1: Toggle again → Q=0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
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
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // S=1, R=0: Set → Q=1
    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=0: Hold → Q stays 1
    swS.setOn(false);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=1: Reset → Q=0
    swR.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
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
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // S=1, R=0: Set → Q=1
    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=0: Hold → Q stays 1
    swS.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // S=0, R=1: Reset → Q=0
    swR.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // S=1, R=1: Invalid state → Q=0, ~Q=0
    swS.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
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
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // T=0: clock pulse → no toggle (hold)
    swT.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // T=1: clock pulse → toggle Q=0→1
    swT.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);

    // T=1: clock pulse → toggle Q=1→0
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);
}

void TestSequentialLogic::testSRFlipFlopWithOneOfSAndROpen()
{
    // An open S or R reads Inactive: the flip-flop sets with R open, and presets and resets with S open
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swClk, swS, swR, swPreset;
    SRFlipFlop setOnly, resetOnly;
    Led ledSet, ledReset;

    builder.add(&swClk, &swS, &swR, &swPreset, &setOnly, &resetOnly, &ledSet, &ledReset);
    builder.connect(&swS, 0, &setOnly, 0);        // S; R open
    builder.connect(&swClk, 0, &setOnly, 1);
    builder.connect(&setOnly, 0, &ledSet, 0);
    builder.connect(&swClk, 0, &resetOnly, 1);    // S open
    builder.connect(&swR, 0, &resetOnly, 2);      // R
    builder.connect(&swPreset, 0, &resetOnly, 3); // ~Preset
    builder.connect(&resetOnly, 0, &ledReset, 0);

    swPreset.setOn(true);
    auto *sim = builder.initSimulation();
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledSet), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Inactive);

    swS.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledSet), Status::Active);

    swPreset.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Active);
    swPreset.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Active);

    swR.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledReset), Status::Inactive);
}

void TestSequentialLogic::testTFlipFlopWithTOpen()
{
    // An open T reads Inactive: the flip-flop presets, holds on a clock edge and clears
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swClk, swPreset, swClear;
    TFlipFlop tff;
    Led ledQ;

    builder.add(&swClk, &swPreset, &swClear, &tff, &ledQ);
    builder.connect(&swClk, 0, &tff, 1);    // T open
    builder.connect(&swPreset, 0, &tff, 2); // ~Preset
    builder.connect(&swClear, 0, &tff, 3);  // ~Clear
    builder.connect(&tff, 0, &ledQ, 0);

    swPreset.setOn(true);
    swClear.setOn(true);
    auto *sim = builder.initSimulation();
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);

    swPreset.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    swPreset.setOn(true);
    sim->update();

    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    swClear.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
}

// ============================================================================
// resetSimState() Tests
// ============================================================================
// resetSimState() is called by WaveformSimulator before each fresh BeWavedDolphin sweep;
// no BeWavedDolphin test circuit happens to include a flip-flop, so these exercise it
// directly: drive Q away from its power-on default, reset, and confirm both the output
// AND the edge-detection state came back cleanly (a subsequent real clock edge still
// behaves correctly, proving the internal m_simLastXxx bookkeeping wasn't left corrupt).
//
// Each test drives an input change (setOn(false) on the data-like input) right after
// resetSimState(), exactly as WaveformSimulator::sweep() always does immediately after
// its own reset loop (it goes straight into driving the first column's input values).
// resetSimState() bypasses setOutputValue()'s change-detection (Simulation::update()'s
// idle-tick skip only wakes on a clock/input-element change), so calling it with no
// following input change is not a scenario any real caller produces.

void TestSequentialLogic::testDFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swD, swClk;
    DFlipFlop dff;
    Led ledQ, ledNQ;

    builder.add(&swD, &swClk, &dff, &ledQ, &ledNQ);
    builder.connect(&swD, 0, &dff, 0);
    builder.connect(&swClk, 0, &dff, 1);
    builder.connect(&dff, 0, &ledQ, 0);
    builder.connect(&dff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    dff.resetSimState();
    swD.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with D=1 must still set Q=1 correctly after the reset.
    swD.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

void TestSequentialLogic::testJKFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swJ, swClk, swK;
    JKFlipFlop jkff;
    Led ledQ, ledNQ;

    builder.add(&swJ, &swClk, &swK, &jkff, &ledQ, &ledNQ);
    builder.connect(&swJ, 0, &jkff, 0);
    builder.connect(&swClk, 0, &jkff, 1);
    builder.connect(&swK, 0, &jkff, 2);
    builder.connect(&jkff, 0, &ledQ, 0);
    builder.connect(&jkff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swJ.setOn(true);
    swK.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    jkff.resetSimState();
    swJ.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with J=1,K=0 must still Set correctly after the reset.
    swJ.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

void TestSequentialLogic::testSRFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swS, swClk, swR;
    SRFlipFlop srff;
    Led ledQ, ledNQ;

    builder.add(&swS, &swClk, &swR, &srff, &ledQ, &ledNQ);
    builder.connect(&swS, 0, &srff, 0);
    builder.connect(&swClk, 0, &srff, 1);
    builder.connect(&swR, 0, &srff, 2);
    builder.connect(&srff, 0, &ledQ, 0);
    builder.connect(&srff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swS.setOn(true);
    swR.setOn(false);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    srff.resetSimState();
    swS.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with S=1,R=0 must still Set correctly after the reset.
    swS.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

void TestSequentialLogic::testTFlipFlopResetSimState()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swT, swClk;
    TFlipFlop tff;
    Led ledQ, ledNQ;

    builder.add(&swT, &swClk, &tff, &ledQ, &ledNQ);
    builder.connect(&swT, 0, &tff, 0);
    builder.connect(&swClk, 0, &tff, 1);
    builder.connect(&tff, 0, &ledQ, 0);
    builder.connect(&tff, 1, &ledNQ, 0);

    auto *sim = builder.initSimulation();
    sim->update();

    swT.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);

    tff.resetSimState();
    swT.setOn(false); // wakes the simulation's idle-tick skip, as a real sweep column would
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Inactive);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Active);

    // A fresh rising edge with T=1 must still toggle correctly after the reset.
    swT.setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, &swClk);
    QCOMPARE(TestUtils::inputStatus(&ledQ), Status::Active);
    QCOMPARE(TestUtils::inputStatus(&ledNQ), Status::Inactive);
}

// ============================================================================
// updateTheme() Tests
// ============================================================================

void TestSequentialLogic::testSequentialElementsUpdateTheme()
{
    // pixmap() is protected, so appearancePreviewPixmap() (backed by the same
    // ElementAppearance the pixmap()/setPixmap() pair reads/writes) stands in for it.
    const QSize previewSize(64, 64);

    DFlipFlop dff;
    QVERIFY(!dff.appearancePreviewPixmap(0, previewSize).isNull());
    dff.updateTheme();
    QVERIFY(!dff.appearancePreviewPixmap(0, previewSize).isNull());

    JKFlipFlop jkff;
    QVERIFY(!jkff.appearancePreviewPixmap(0, previewSize).isNull());
    jkff.updateTheme();
    QVERIFY(!jkff.appearancePreviewPixmap(0, previewSize).isNull());

    SRFlipFlop srff;
    QVERIFY(!srff.appearancePreviewPixmap(0, previewSize).isNull());
    srff.updateTheme();
    QVERIFY(!srff.appearancePreviewPixmap(0, previewSize).isNull());

    TFlipFlop tff;
    QVERIFY(!tff.appearancePreviewPixmap(0, previewSize).isNull());
    tff.updateTheme();
    QVERIFY(!tff.appearancePreviewPixmap(0, previewSize).isNull());

    DLatch dlatch;
    QVERIFY(!dlatch.appearancePreviewPixmap(0, previewSize).isNull());
    dlatch.updateTheme();
    QVERIFY(!dlatch.appearancePreviewPixmap(0, previewSize).isNull());

    SRLatch srlatch;
    QVERIFY(!srlatch.appearancePreviewPixmap(0, previewSize).isNull());
    srlatch.updateTheme();
    QVERIFY(!srlatch.appearancePreviewPixmap(0, previewSize).isNull());
}

namespace TestSequentialLogicHelpers {

/// What \a element draws, unselected: its picture, as paint() gives it.
QImage drawn(Element &element)
{
    const bool selected = element.isSelected();
    element.setSelected(false);
    QImage image(96, 96, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    const QStyleOptionGraphicsItem option;
    element.paint(&painter, &option, nullptr);
    painter.end();
    element.setSelected(selected);
    return image;
}

/// Makes each latch and flip-flop, whose picture is the theme's.
QList<std::function<Element *()>> themedTypes()
{
    return {
        [] { return new DFlipFlop; }, [] { return new JKFlipFlop; }, [] { return new SRFlipFlop; },
        [] { return new TFlipFlop; }, [] { return new DLatch; }, [] { return new SRLatch; },
    };
}

/// What a new element made by \a make draws in \a theme.
QImage drawnNew(const std::function<Element *()> &make, const Theme theme)
{
    ThemeManager::setTheme(theme);
    std::unique_ptr<Element> element(make());
    return drawn(*element);
}

} // namespace TestSequentialLogicHelpers

using TestSequentialLogicHelpers::drawn;
using TestSequentialLogicHelpers::drawnNew;
using TestSequentialLogicHelpers::themedTypes;

void TestSequentialLogic::testThemedPictureFollowsTheThemeThroughARefresh()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    for (const auto &make : themedTypes()) {
        const QImage dark = drawnNew(make, Theme::Dark);
        ThemeManager::setTheme(Theme::Light);
        std::unique_ptr<Element> element(make());
        const QString name = element->objectName();
        QVERIFY2(drawn(*element) != dark, qPrintable(name));

        ThemeManager::setTheme(Theme::Dark);
        element->updateTheme();
        QVERIFY2(drawn(*element) == dark, qPrintable(name));
        element->refresh();
        QVERIFY2(drawn(*element) == dark, qPrintable(name));
    }
}

void TestSequentialLogic::testThemeSwitchKeepsACustomAppearance()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString png = dir.filePath("custom.png");
    QImage red(64, 64, QImage::Format_ARGB32);
    red.fill(Qt::red);
    QVERIFY(red.save(png));

    for (const auto &make : themedTypes()) {
        const QImage dark = drawnNew(make, Theme::Dark);
        ThemeManager::setTheme(Theme::Light);
        std::unique_ptr<Element> element(make());
        const QString name = element->objectName();
        element->setAppearance(false, png);
        const QImage custom = drawn(*element);

        ThemeManager::setTheme(Theme::Dark);
        element->updateTheme();
        QVERIFY2(drawn(*element) == custom, qPrintable(name));
        element->refresh();
        QVERIFY2(drawn(*element) == custom, qPrintable(name));
        QCOMPARE(element->externalFiles(), QStringList{png});

        // Back to the built-in appearance, it is the dark one
        element->setAppearance(true, {});
        QVERIFY2(drawn(*element) == dark, qPrintable(name));
    }
}

void TestSequentialLogic::testUndoAfterAThemeSwitchShowsTheCurrentTheme()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    QList<QImage> dark;
    for (const auto &make : themedTypes()) {
        dark.append(drawnNew(make, Theme::Dark));
    }

    // A label edit of each, recorded while the theme is light
    ThemeManager::setTheme(Theme::Light);
    Scene scene;
    QList<Element *> elements;
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (const auto &make : themedTypes()) {
        auto *element = make();
        element->setPos(0, 128 * static_cast<int>(elements.size()));
        scene.addItem(element);
        element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        element->setLabel("Q");
        elements.append(element);
    }
    scene.receiveCommand(new UpdateCommand(elements, oldData, &scene));

    ThemeManager::setTheme(Theme::Dark);
    scene.updateTheme();
    for (int i = 0; i < elements.size(); ++i) {
        QVERIFY2(drawn(*elements.at(i)) == dark.at(i), qPrintable(elements.at(i)->objectName()));
    }

    scene.undoStack()->undo();
    for (int i = 0; i < elements.size(); ++i) {
        auto *element = elements.at(i);
        QCOMPARE(element->label(), QString());
        QVERIFY2(drawn(*element) == dark.at(i), qPrintable(element->objectName()));
        QVERIFY2(element->externalFiles().isEmpty(), qPrintable(element->objectName()));
    }

    scene.undoStack()->redo();
    for (int i = 0; i < elements.size(); ++i) {
        auto *element = elements.at(i);
        QCOMPARE(element->label(), QString("Q"));
        QVERIFY2(drawn(*element) == dark.at(i), qPrintable(element->objectName()));
    }
}
