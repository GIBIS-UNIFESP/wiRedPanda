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
    // Each input is either wired to a switch, or left open with a level for its default: every mix of the two, and every
    // level the switches take, is settled by master and compared with the reference's value of the same levels
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
