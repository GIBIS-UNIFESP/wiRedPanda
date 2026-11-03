// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestRandomCircuit.h"

#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/RandomCircuit.h"
#include "Tests/Common/TraceEngine.h"

void TestRandomCircuit::testSameSeedBuildsSameCircuit()
{
    QStringList distinct;
    for (quint32 seed = 1; seed <= 20; ++seed) {
        SeededChoices first(seed);
        SeededChoices second(seed);
        const auto a = makeRandomCircuit(first);
        const auto b = makeRandomCircuit(second);
        const QStringList description = describeRandomCircuit(a);
        QCOMPARE(describeRandomCircuit(b), description);
        distinct << description.join('\n');
    }
    distinct.removeDuplicates();
    QVERIFY2(distinct.size() > 10, "different seeds should build different circuits");
}

void TestRandomCircuit::testBuildingBlocksLoad()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QStringList blocks = buildingBlockFiles(dir.path());
    QCOMPARE(blocks.size(), 8);
    for (const QString &path : blocks) {
        QVERIFY2(QFileInfo::exists(path), qPrintable(path));
        auto *ic = new IC();
        ic->loadFile(path, dir.path());
        QVERIFY2(ic->inputSize() > 0, qPrintable(QString("%1 has no inputs").arg(path)));
        QVERIFY2(ic->outputSize() > 0, qPrintable(QString("%1 has no outputs").arg(path)));
        delete ic;
    }
}

void TestRandomCircuit::testEveryFeatureIsGenerated()
{
    // Each feature must appear in some circuit of the 300 seeds, or a walk over them never meets it.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QStringList blocks = buildingBlockFiles(dir.path());
    RandomCircuitFeatures total;
    for (quint32 seed = 1; seed <= 300; ++seed) {
        SeededChoices choices(seed);
        const auto circuit = makeRandomCircuit(choices, blocks);
        total.openInputs += circuit.features.openInputs;
        total.doubleDriven += circuit.features.doubleDriven;
        total.feedbackWires += circuit.features.feedbackWires;
        total.lockedClocks += circuit.features.lockedClocks;
        total.memoryElements += circuit.features.memoryElements;
        total.gateElements += circuit.features.gateElements;
        total.muxElements += circuit.features.muxElements;
        total.truthTables += circuit.features.truthTables;
        total.delayedElements += circuit.features.delayedElements;
        total.wirelessNodes += circuit.features.wirelessNodes;
        total.wiredReceivers += circuit.features.wiredReceivers;
        total.memoryClockOwn += circuit.features.memoryClockOwn;
        total.memoryClockInput += circuit.features.memoryClockInput;
        total.memoryClockClock += circuit.features.memoryClockClock;
        total.icInstances += circuit.features.icInstances;
        total.rippleChains += circuit.features.rippleChains;
    }
    QVERIFY2(total.openInputs > 0, "no input was left open");
    QVERIFY2(total.doubleDriven > 0, "no input had two drivers");
    QVERIFY2(total.feedbackWires > 0, "no circuit had feedback");
    QVERIFY2(total.lockedClocks > 0, "no clock was locked");
    QVERIFY2(total.memoryElements > 0, "no flip-flop or latch was generated");
    QVERIFY2(total.gateElements > 0, "no gate was generated");
    QVERIFY2(total.muxElements > 0, "no multiplexer was generated");
    QVERIFY2(total.truthTables > 0, "no truth table was generated");
    QVERIFY2(total.delayedElements > 0, "no element had a temporal delay");
    QVERIFY2(total.wirelessNodes > 0, "no wireless node was generated");
    QVERIFY2(total.wiredReceivers > 0, "no receiver was wired");
    QVERIFY2(total.memoryClockOwn > 0, "no memory clock came from its own output");
    QVERIFY2(total.memoryClockInput > 0, "no memory clock came from an input");
    QVERIFY2(total.memoryClockClock > 0, "no memory clock came from a clock");
    QVERIFY2(total.icInstances > 0, "no building-block IC was instanced");
    QVERIFY2(total.rippleChains > 0, "no ripple chain past the round budget was generated");
}

void TestRandomCircuit::testWalksRunOnMasterEngine()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QStringList blocks = buildingBlockFiles(dir.path());
    for (quint32 seed = 1; seed <= 100; ++seed) {
        SeededChoices build(seed);
        auto circuit = makeRandomCircuit(build, blocks);
        const auto engine = makeMasterTraceEngine(circuit.workspace->scene());
        QVERIFY2(!engine->timerRuns(), "the simulation's timer must not run during a walk");
        SeededChoices walk(seed + 1000);
        const QStringList lines = randomCircuitWalk(circuit, *engine, walk, 40);
        QCOMPARE(lines.size(), 40);

        // The same seed builds the same circuit and walks it the same way.
        SeededChoices buildAgain(seed);
        auto again = makeRandomCircuit(buildAgain, blocks);
        const auto engineAgain = makeMasterTraceEngine(again.workspace->scene());
        SeededChoices walkAgain(seed + 1000);
        QCOMPARE(randomCircuitWalk(again, *engineAgain, walkAgain, 40), lines);
    }
}
