// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestPriorities.h"

#include "App/Core/Priorities.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Wiring/Connection.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

// Priority Calculation Tests

void TestPriorities::testPriorityBasic()
{
    // Test: Priority is calculated based on distance from outputs
    // SW → AND → LED: LED=1, AND=2, SW=3
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    Led led;

    builder.add(&sw, &andGate, &led);
    builder.connect(&sw, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led, 0);

    QHash<GraphicElement *, QVector<GraphicElement *>> successors;
    QVector<GraphicElement *> elements = {&sw, &andGate, &led};
    for (auto *elm : elements) {
        for (auto *port : elm->outputs()) {
            for (auto *conn : port->connections()) {
                if (auto *endPort = conn->endPort()) {
                    if (auto *succ = endPort->graphicElement()) {
                        if (!successors[elm].contains(succ)) {
                            successors[elm].append(succ);
                        }
                    }
                }
            }
        }
    }

    QHash<GraphicElement *, int> priorities;
    calculatePriorities(elements, successors, priorities);

    int swPriority = priorities.value(&sw);
    int andPriority = priorities.value(&andGate);
    int ledPriority = priorities.value(&led);

    // Priority = max(successor priorities) + 1
    // LED (output, no successors): max=0 → priority 1
    // AND (successor: LED, priority 1): max=1 → priority 2
    // SW (successor: AND, priority 2): max=2 → priority 3
    QCOMPARE(ledPriority, 1);
    QCOMPARE(andPriority, 2);
    QCOMPARE(swPriority, 3);
}

void TestPriorities::testPriorityMemoization()
{
    // Test: Priority memoization - same element shouldn't recalculate
    // Create: SW1 → AND → LED
    //         SW2 ↗
    // Both SW1 and SW2 connect to AND, so AND's priority calculated only once
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);

    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    QHash<GraphicElement *, QVector<GraphicElement *>> successors;
    QVector<GraphicElement *> elements = {&sw1, &sw2, &andGate, &led};
    for (auto *elm : elements) {
        for (auto *port : elm->outputs()) {
            for (auto *conn : port->connections()) {
                if (auto *endPort = conn->endPort()) {
                    if (auto *succ = endPort->graphicElement()) {
                        if (!successors[elm].contains(succ)) {
                            successors[elm].append(succ);
                        }
                    }
                }
            }
        }
    }

    QHash<GraphicElement *, int> priorities;
    calculatePriorities(elements, successors, priorities);

    int sw1Priority = priorities.value(&sw1);
    int sw2Priority = priorities.value(&sw2);

    // AND's priority should be computed
    QVERIFY(priorities.contains(&andGate));

    // SW1 and SW2 should have same priority (same distance to outputs)
    QCOMPARE(sw1Priority, sw2Priority);
}

void TestPriorities::testPriorityTransitiveDiamond()
{
    // Regression test (F1): A → B, A → C, C → B is cycle-free, so priorities
    // must be the longest path to a sink regardless of iteration order:
    // B (sink) = 1, C = 2, A = 3.
    //
    // The old algorithm treated C's pending sibling B (pushed by A but not yet
    // expanded) as a feedback edge and produced {B:1, C:1, A:2} for the
    // iteration order [A, B, C] — a consumer (B) tied with its producer (C).
    int a, b, c;
    QHash<int *, QVector<int *>> successors;
    successors[&a] = {&b, &c};
    successors[&c] = {&b};

    const QVector<QVector<int *>> orders = {
        {&a, &b, &c},
        {&b, &c, &a},
        {&a, &c, &b},
    };

    for (const auto &elements : orders) {
        QHash<int *, int> priorities;
        calculatePriorities(elements, successors, priorities);

        QCOMPARE(priorities.value(&b), 1);
        QCOMPARE(priorities.value(&c), 2);
        QCOMPARE(priorities.value(&a), 3);
    }
}

void TestPriorities::testPriorityCycleSharedPriority()
{
    // A → B → C → B (B and C form a cycle, A feeds it): cycle members occupy
    // a contiguous band strictly below their upstream feeder, ordered by
    // discovery — the loop entry B evaluates before C, so the external input
    // propagates through the loop in flow order (latch races settle the way
    // real hardware would).
    int a, b, c;
    QVector<int *> elements = {&a, &b, &c};
    QHash<int *, QVector<int *>> successors;
    successors[&a] = {&b};
    successors[&b] = {&c};
    successors[&c] = {&b};

    QHash<int *, int> priorities;
    calculatePriorities(elements, successors, priorities);

    QCOMPARE(priorities.value(&c), 1);
    QCOMPARE(priorities.value(&b), 2);
    QCOMPARE(priorities.value(&a), 3);
}

// Feedback Detection Tests

void TestPriorities::testFeedbackNoCycle()
{
    // A → B → C (linear chain, no cycle)
    int a, b, c;
    QVector<int *> elements = {&a, &b, &c};
    QHash<int *, QVector<int *>> successors;
    successors[&a].append(&b);
    successors[&b].append(&c);

    const auto feedback = findFeedbackNodes(elements, successors);
    QVERIFY(feedback.isEmpty());
}

void TestPriorities::testFeedbackSimpleCycle()
{
    // A → B → A (both should be feedback)
    int a, b;
    QVector<int *> elements = {&a, &b};
    QHash<int *, QVector<int *>> successors;
    successors[&a].append(&b);
    successors[&b].append(&a);

    const auto feedback = findFeedbackNodes(elements, successors);
    QCOMPARE(feedback.size(), 2);
    QVERIFY(feedback.contains(&a));
    QVERIFY(feedback.contains(&b));
}

void TestPriorities::testFeedbackSelfLoop()
{
    // A → A (self-loop)
    int a;
    QVector<int *> elements = {&a};
    QHash<int *, QVector<int *>> successors;
    successors[&a].append(&a);

    const auto feedback = findFeedbackNodes(elements, successors);
    QCOMPARE(feedback.size(), 1);
    QVERIFY(feedback.contains(&a));
}

void TestPriorities::testFeedbackCycleWithTail()
{
    // A → B → C → B (B and C in cycle, A is not)
    int a, b, c;
    QVector<int *> elements = {&a, &b, &c};
    QHash<int *, QVector<int *>> successors;
    successors[&a].append(&b);
    successors[&b].append(&c);
    successors[&c].append(&b);

    const auto feedback = findFeedbackNodes(elements, successors);
    QCOMPARE(feedback.size(), 2);
    QVERIFY(feedback.contains(&b));
    QVERIFY(feedback.contains(&c));
    QVERIFY(!feedback.contains(&a));
}

void TestPriorities::testFeedbackMultipleCycles()
{
    // Two independent cycles: A ↔ B, C ↔ D
    int a, b, c, d;
    QVector<int *> elements = {&a, &b, &c, &d};
    QHash<int *, QVector<int *>> successors;
    successors[&a].append(&b);
    successors[&b].append(&a);
    successors[&c].append(&d);
    successors[&d].append(&c);

    const auto feedback = findFeedbackNodes(elements, successors);
    QCOMPARE(feedback.size(), 4);
    QVERIFY(feedback.contains(&a));
    QVERIFY(feedback.contains(&b));
    QVERIFY(feedback.contains(&c));
    QVERIFY(feedback.contains(&d));
}

void TestPriorities::testFeedbackTriangleCycle()
{
    // A → B → C → A (all three in cycle)
    int a, b, c;
    QVector<int *> elements = {&a, &b, &c};
    QHash<int *, QVector<int *>> successors;
    successors[&a].append(&b);
    successors[&b].append(&c);
    successors[&c].append(&a);

    const auto feedback = findFeedbackNodes(elements, successors);
    QCOMPARE(feedback.size(), 3);
    QVERIFY(feedback.contains(&a));
    QVERIFY(feedback.contains(&b));
    QVERIFY(feedback.contains(&c));
}

void TestPriorities::testFeedbackDiamond()
{
    // A → B, A → C, B → D, C → D (diamond, no cycle)
    int a, b, c, d;
    QVector<int *> elements = {&a, &b, &c, &d};
    QHash<int *, QVector<int *>> successors;
    successors[&a].append(&b);
    successors[&a].append(&c);
    successors[&b].append(&d);
    successors[&c].append(&d);

    const auto feedback = findFeedbackNodes(elements, successors);
    QVERIFY(feedback.isEmpty());
}
