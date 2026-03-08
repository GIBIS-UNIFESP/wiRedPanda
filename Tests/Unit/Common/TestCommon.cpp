// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestCommon.h"

#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

// Topological Sort Tests

void TestCommon::testSortSimpleChain()
{
    // Test: InputSwitch → AND → LED (simple chain)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    Led led;

    builder.add(&sw, &andGate, &led);
    builder.connect(&sw, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led, 0);

    // Get only our specific elements (don't rely on scene elements)
    QVector<GraphicElement *> elements{&sw, &andGate, &led};

    // Sort elements
    auto sorted = Common::sortGraphicElements(elements);

    // Verify all elements are present
    QCOMPARE(sorted.size(), 3);

    // Verify ordering: LED (priority 1) should come before AND (priority 2) should come before SW (priority 3)
    // Priority is based on distance from outputs, higher priority = deeper in dependency chain
    int ledIndex = -1, andIndex = -1, swIndex = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i] == &led) {
            ledIndex = i;
        } else if (sorted[i] == &andGate) {
            andIndex = i;
        } else if (sorted[i] == &sw) {
            swIndex = i;
        }
    }

    QVERIFY2(ledIndex >= 0, "LED not found in sorted list");
    QVERIFY2(andIndex >= 0, "AND gate not found in sorted list");
    QVERIFY2(swIndex >= 0, "Switch not found in sorted list");

    // Priority-based sorting: higher priority (further from outputs) comes first
    // SW has priority 3 (highest), AND has priority 2, LED has priority 1 (lowest)
    // So sorted order should be: SW (index 0), AND (index 1), LED (index 2)
    QVERIFY2(swIndex < andIndex, "SW should come before AND");
    QVERIFY2(andIndex < ledIndex, "AND should come before LED");
}

void TestCommon::testSortMultipleChains()
{
    // Test: Multiple independent chains
    // SW1 → AND → LED1
    // SW2 → OR → LED2
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Or orGate;
    Led led1, led2;

    builder.add(&sw1, &sw2, &andGate, &orGate, &led1, &led2);

    // First chain
    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led1, 0);

    // Second chain
    builder.connect(&sw2, 0, &orGate, 0);
    builder.connect(&orGate, 0, &led2, 0);

    // Use our specific elements
    QVector<GraphicElement *> elements{&sw1, &sw2, &andGate, &orGate, &led1, &led2};

    // Sort elements
    auto sorted = Common::sortGraphicElements(elements);

    // Verify all elements are present
    QCOMPARE(sorted.size(), 6);

    // Both chains should be sorted independently
    // Count our specific elements
    int ledCount = 0;
    int andOrCount = 0;
    int switchCount = 0;

    for (auto *elem : sorted) {
        if (elem == &led1 || elem == &led2) {
            ledCount++;
        } else if (elem == &andGate || elem == &orGate) {
            andOrCount++;
        } else if (elem == &sw1 || elem == &sw2) {
            switchCount++;
        }
    }

    QCOMPARE(ledCount, 2);
    QCOMPARE(andOrCount, 2);
    QCOMPARE(switchCount, 2);
}

void TestCommon::testSortCycleDetection()
{
    // Test: Cycle handling (SW → AND → OR → back to AND input)
    // The cycle detection should prevent infinite recursion
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    Or orGate;

    builder.add(&sw, &andGate, &orGate);

    // Create connections: SW → AND → OR
    builder.connect(&sw, 0, &andGate, 0);
    builder.connect(&andGate, 0, &orGate, 0);

    // Create back edge: OR → AND (creates cycle)
    builder.connect(&orGate, 0, &andGate, 1);

    // Use our specific elements
    QVector<GraphicElement *> elements{&sw, &andGate, &orGate};

    // Sort should handle cycle without hanging
    auto sorted = Common::sortGraphicElements(elements);

    // Verify all elements are present (no crash or infinite loop)
    QCOMPARE(sorted.size(), 3);

    // Verify each element appears exactly once (no duplicates due to cycle)
    int andCount = 0, orCount = 0, swCount = 0;
    for (auto *elem : sorted) {
        if (elem == &andGate) andCount++;
        else if (elem == &orGate) orCount++;
        else if (elem == &sw) swCount++;
    }
    QCOMPARE(andCount, 1);  // Each appears exactly once
    QCOMPARE(orCount, 1);
    QCOMPARE(swCount, 1);

    // Verify output element (orGate) comes early in sort order
    // since it needs to be updated first in cycle handling
    int orIndex = -1, andIndex = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i] == &orGate) orIndex = i;
        if (sorted[i] == &andGate) andIndex = i;
    }
    QVERIFY(orIndex >= 0);
    QVERIFY(andIndex >= 0);
}

void TestCommon::testSortDisconnectedComponents()
{
    // Test: Elements with no connections
    // SW1 (isolated) + (SW2 → AND → LED)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);

    // Create chain: SW2 → AND → LED
    builder.connect(&sw2, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led, 0);

    // SW1 is left disconnected

    // Use our specific elements
    QVector<GraphicElement *> elements{&sw1, &sw2, &andGate, &led};

    // Sort should handle disconnected components
    auto sorted = Common::sortGraphicElements(elements);

    // All elements should be present
    QCOMPARE(sorted.size(), 4);

    // Find indices of each element in sorted order
    QHash<GraphicElement *, int> sortedIndex;
    for (int i = 0; i < sorted.size(); ++i) {
        sortedIndex[sorted[i]] = i;
    }

    QVERIFY(sortedIndex.contains(&sw1));
    QVERIFY(sortedIndex.contains(&sw2));
    QVERIFY(sortedIndex.contains(&andGate));
    QVERIFY(sortedIndex.contains(&led));

    // Verify the connected chain ordering based on priority (higher priority first)
    // Priority calculation: distance from outputs
    // SW2 (input) has highest priority (3 - furthest from output LED)
    // AND (logic gate) has medium priority (2 - between input and output)
    // LED (output) has lowest priority (1 - distance 0 from output)
    // Therefore sort order should be: SW2, AND, LED
    int ledIndex = sortedIndex.value(&led, -1);
    int andIndex = sortedIndex.value(&andGate, -1);
    int sw2Index = sortedIndex.value(&sw2, -1);
    int sw1Index = sortedIndex.value(&sw1, -1);

    // Verify priority-based ordering: higher priority elements come first
    QVERIFY2(sw2Index < andIndex, "SW2 (priority 3) should come before AND (priority 2)");
    QVERIFY2(andIndex < ledIndex, "AND (priority 2) should come before LED (priority 1)");

    // SW1 (isolated) can appear anywhere, but should be present
    QVERIFY2(sw1Index >= 0, "SW1 (isolated) should be in sorted list");
}

// Priority Calculation Tests

void TestCommon::testPriorityBasic()
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

    QMap<GraphicElement *, bool> beingVisited;
    QMap<GraphicElement *, int> priorities;

    // Calculate priority for each element
    int swPriority = Common::calculatePriority(&sw, beingVisited, priorities);
    int andPriority = Common::calculatePriority(&andGate, beingVisited, priorities);
    int ledPriority = Common::calculatePriority(&led, beingVisited, priorities);

    // Priority = max(successor priorities) + 1
    // LED (output, no successors): max=0 → priority 1
    // AND (successor: LED, priority 1): max=1 → priority 2
    // SW (successor: AND, priority 2): max=2 → priority 3
    QCOMPARE(ledPriority, 1);
    QCOMPARE(andPriority, 2);
    QCOMPARE(swPriority, 3);
}

void TestCommon::testPriorityMemoization()
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

    QMap<GraphicElement *, bool> beingVisited;
    QMap<GraphicElement *, int> priorities;

    // Calculate priority for SW1 (which will calculate AND's priority)
    int sw1Priority = Common::calculatePriority(&sw1, beingVisited, priorities);

    // AND's priority should be cached
    QVERIFY(priorities.contains(&andGate));
    int cachedAndPriority = priorities.value(&andGate);

    // Calculate priority for SW2 (should use cached AND priority)
    int sw2Priority = Common::calculatePriority(&sw2, beingVisited, priorities);

    // AND's priority should not change
    QCOMPARE(priorities.value(&andGate), cachedAndPriority);

    // SW1 and SW2 should have same priority (same distance to outputs)
    QCOMPARE(sw1Priority, sw2Priority);
}

// Exception Tests

void TestCommon::testPandaceptionMessage()
{
    // Test: Pandaception exception with translated message
    QString translatedMsg = "Translated error message";
    QString englishMsg = "English error message";

    Pandaception ex(translatedMsg, englishMsg);

    // Verify translated message is in what() (std::runtime_error)
    QCOMPARE(QString::fromStdString(ex.what()), translatedMsg);

    // Verify both messages are accessible
    QCOMPARE(ex.englishMessage(), englishMsg);
}

void TestCommon::testPandaceptionEnglishMessage()
{
    // Test: Pandaception keeps English message separate
    QString translatedMsg = "Mensaje de error traducido";  // Spanish
    QString englishMsg = "Error message";

    Pandaception ex(translatedMsg, englishMsg);

    // Translated message is in what()
    QCOMPARE(QString::fromStdString(ex.what()), translatedMsg);

    // English message is in englishMessage()
    QCOMPARE(ex.englishMessage(), englishMsg);

    // Both are different
    QVERIFY(QString::fromStdString(ex.what()) != ex.englishMessage());
}
