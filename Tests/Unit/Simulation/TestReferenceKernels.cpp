// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestReferenceKernels.h"

#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/ReferenceKernels.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Common/TraceEngine.h"

namespace {

/// The gates the kernels are checked for, each with the input counts they take.
struct GateCase
{
    ElementType type;
    int inputs;
};

const QVector<GateCase> &gateCases()
{
    static const QVector<GateCase> cases = {
        {ElementType::Not, 1},  {ElementType::And, 2}, {ElementType::And, 3},
        {ElementType::Or, 2},   {ElementType::Or, 3},  {ElementType::Nand, 2}, {ElementType::Nor, 3},
        {ElementType::Xor, 2},  {ElementType::Xor, 3}, {ElementType::Xnor, 2}, {ElementType::Xnor, 3},
        {ElementType::And, 4},  {ElementType::Or, 4},   {ElementType::Xor, 4},  {ElementType::Nor, 4},
    };
    return cases;
}

/// Every vector of \a count values from the four: each position takes Inactive, Active, Unknown or Error.
QVector<QVector<Status>> everyVector(const int count)
{
    const QVector<Status> values = {Status::Inactive, Status::Active, Status::Unknown, Status::Error};
    QVector<QVector<Status>> vectors = {{}};
    for (int position = 0; position < count; ++position) {
        QVector<QVector<Status>> grown;
        for (const QVector<Status> &prefix : std::as_const(vectors)) {
            for (const Status value : values) {
                QVector<Status> next = prefix;
                next.append(value);
                grown.append(next);
            }
        }
        vectors = grown;
    }
    return vectors;
}

} // namespace

void TestReferenceKernels::testLevelsMatchMaster()
{
    // Each input is either wired to a switch, or left open with a level for its default: every mix of the two, and
    // every level the switches take, is settled by master and compared with the reference's value of the same levels
    for (const GateCase &gateCase : gateCases()) {
        for (int openMask = 0; openMask < (1 << gateCase.inputs); ++openMask) {
            for (int defaultMask = 0; defaultMask < (1 << gateCase.inputs); ++defaultMask) {
                auto workspace = TestUtils::createWorkspace();
                CircuitBuilder builder(workspace->scene());
                auto *gate = ElementFactory::buildElement(gateCase.type);
                TestUtils::applyPortCounts(gate, gateCase.inputs, 1);
                builder.addElement(gate);
                QVector<Element *> switches(gateCase.inputs, nullptr);
                QVector<Status> openValues(gateCase.inputs, Status::Unknown);
                for (int input = 0; input < gateCase.inputs; ++input) {
                    if ((openMask >> input) & 1) {
                        const Status level = ((defaultMask >> input) & 1) ? Status::Active : Status::Inactive;
                        gate->inputPort(input)->setDefaultStatus(level);
                        openValues[input] = level;
                        continue;
                    }
                    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
                    builder.add(source);
                    builder.connect(source, 0, gate, input);
                    switches[input] = source;
                }

                const auto engine = makeMasterTraceEngine(workspace->scene());
                for (int mask = 0; mask < (1 << gateCase.inputs); ++mask) {
                    QVector<Status> levels;
                    for (int input = 0; input < gateCase.inputs; ++input) {
                        const bool level = ((mask >> input) & 1) != 0;
                        if (switches.at(input)) {
                            switches.at(input)->setOn(level);
                            levels.append(level ? Status::Active : Status::Inactive);
                        } else {
                            levels.append(openValues.at(input));
                        }
                    }
                    engine->step();
                    QCOMPARE(engine->observed(gate).first(), referenceGate(gateCase.type, levels));
                }
            }
        }
    }
}

void TestReferenceKernels::testRuleIsMonotoneInTheFlatOrder()
{
    // Raising an input in the flat order never lowers the gate's output: for every pair of vectors one below the other,
    // the output of the lower is below the output of the higher
    int pairs = 0;
    for (const GateCase &gateCase : gateCases()) {
        const QVector<QVector<Status>> vectors = everyVector(gateCase.inputs);
        for (const QVector<Status> &lower : vectors) {
            for (const QVector<Status> &higher : vectors) {
                bool below = true;
                for (int input = 0; input < gateCase.inputs; ++input) {
                    below = below && atMostInFlatOrder(lower.at(input), higher.at(input));
                }
                if (!below) {
                    continue;
                }
                ++pairs;
                QVERIFY(atMostInFlatOrder(referenceGate(gateCase.type, lower), referenceGate(gateCase.type, higher)));
            }
        }
    }
    QVERIFY(pairs > 0);
}

void TestReferenceKernels::testNonLevelsReadBothWays()
{
    // A level that decides the gate keeps it; a level that leaves it open lets the open input decide
    QCOMPARE(referenceGate(ElementType::And, {Status::Active, Status::Unknown}), Status::Unknown);
    QCOMPARE(referenceGate(ElementType::And, {Status::Inactive, Status::Unknown}), Status::Inactive);
    QCOMPARE(referenceGate(ElementType::Or, {Status::Active, Status::Unknown}), Status::Active);
    QCOMPARE(referenceGate(ElementType::Xor, {Status::Active, Status::Unknown}), Status::Unknown);
    // Error alone leaves a disagreement an Error, and an Unknown beside it wins
    QCOMPARE(referenceGate(ElementType::And, {Status::Active, Status::Error}), Status::Error);
    QCOMPARE(referenceGate(ElementType::And, {Status::Unknown, Status::Error}), Status::Unknown);
    QCOMPARE(referenceGate(ElementType::Not, {Status::Error}), Status::Error);
    QCOMPARE(referenceGate(ElementType::Nor, {Status::Inactive, Status::Error}), Status::Error);
}

void TestReferenceKernels::testMemoryIsMonotoneOffTheKeptClock()
{
    // Stored outputs are dimensions too: every vector of the four values, with the flip-flop's clock pair included. A
    // lower vector whose clock is kept (its last level and now the same status) is left out, as §4.5 allows
    int pairs = 0;
    const QVector<QVector<Status>> flipFlop = everyVector(4);
    for (const QVector<Status> &lower : flipFlop) {
        if (lower.at(1) == lower.at(2)) {
            continue;
        }
        for (const QVector<Status> &higher : flipFlop) {
            bool below = true;
            for (int input = 0; input < 4; ++input) {
                below = below && atMostInFlatOrder(lower.at(input), higher.at(input));
            }
            if (!below) {
                continue;
            }
            ++pairs;
            QVERIFY(atMostInFlatOrder(referenceDFlipFlop(lower), referenceDFlipFlop(higher)));
        }
    }
    QVERIFY(pairs > 0);

    // The D latch has no clock pair to leave out: enable, data and stored output, all monotone
    const QVector<QVector<Status>> latch = everyVector(3);
    for (const QVector<Status> &lower : latch) {
        for (const QVector<Status> &higher : latch) {
            bool below = true;
            for (int input = 0; input < 3; ++input) {
                below = below && atMostInFlatOrder(lower.at(input), higher.at(input));
            }
            if (below) {
                QVERIFY(atMostInFlatOrder(referenceDLatch(lower), referenceDLatch(higher)));
            }
        }
    }
}

void TestReferenceKernels::testMemoryHoldsLoadsAndKeepsItsClock()
{
    // A rising edge with a level clock loads the data; no edge holds the stored output
    QCOMPARE(referenceDFlipFlop({Status::Active, Status::Active, Status::Inactive, Status::Inactive}), Status::Active);
    QCOMPARE(referenceDFlipFlop({Status::Active, Status::Inactive, Status::Inactive, Status::Active}), Status::Active);
    QCOMPARE(referenceDFlipFlop({Status::Active, Status::Active, Status::Active, Status::Inactive}), Status::Inactive);
    // A clock kept at Unknown has no edge, so the stored output holds
    QCOMPARE(referenceDFlipFlop({Status::Active, Status::Unknown, Status::Unknown, Status::Inactive}),
             Status::Inactive);
    // A clock that may have risen decides the output by its Unknown level: the stored output and the data disagree
    QCOMPARE(referenceDFlipFlop({Status::Active, Status::Active, Status::Unknown, Status::Inactive}), Status::Unknown);
    // The latch is transparent while enabled and holds otherwise
    QCOMPARE(referenceDLatch({Status::Active, Status::Active, Status::Inactive}), Status::Active);
    QCOMPARE(referenceDLatch({Status::Inactive, Status::Active, Status::Inactive}), Status::Inactive);
}

void TestReferenceKernels::testMemoryMatchesMasterOnStableData()
{
    // Data is held at one level for a whole run, so no data change falls in an edge's round. There §2.6 gives Unknown
    // where the old and new data differ, and master samples the previous step instead: that case is held for the user's
    // ruling. The clock follows every sequence of levels after a first step that keeps it Inactive; the reference is
    // given each step's clock, the clock's last level and the stored output that master itself last showed.
    constexpr int steps = 4;
    for (int data = 0; data < 2; ++data) {
        for (int sequence = 0; sequence < (1 << steps); ++sequence) {
            auto workspace = TestUtils::createWorkspace();
            CircuitBuilder builder(workspace->scene());
            auto *flipFlop = ElementFactory::buildElement(ElementType::DFlipFlop);
            builder.addElement(flipFlop);
            auto *dataSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
            auto *clockSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
            builder.add(dataSwitch, clockSwitch);
            builder.connect(dataSwitch, 0, flipFlop, 0);
            builder.connect(clockSwitch, 0, flipFlop, 1);
            // Its preset and clear are left open, held at their inactive level
            for (int input = 2; input < flipFlop->inputSize(); ++input) {
                flipFlop->inputPort(input)->setDefaultStatus(Status::Active);
            }
            dataSwitch->setOn(data != 0);

            const auto engine = makeMasterTraceEngine(workspace->scene());
            clockSwitch->setOn(false);
            engine->step();
            Status previousClock = Status::Inactive;
            Status stored = engine->observed(flipFlop).first();
            const Status dataLevel = (data != 0) ? Status::Active : Status::Inactive;
            for (int step = 0; step < steps; ++step) {
                const bool level = ((sequence >> step) & 1) != 0;
                clockSwitch->setOn(level);
                engine->step();
                const Status clock = level ? Status::Active : Status::Inactive;
                const Status expected = referenceDFlipFlop({dataLevel, clock, previousClock, stored});
                QCOMPARE(engine->observed(flipFlop).first(), expected);
                stored = expected;
                previousClock = clock;
            }
        }
    }
}
