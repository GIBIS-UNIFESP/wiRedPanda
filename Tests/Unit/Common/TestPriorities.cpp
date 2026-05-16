// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestPriorities.h"

#include "App/Core/Priorities.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Nodes/QNEConnection.h"
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
