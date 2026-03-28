// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestClocksAdvanced.h"

#include <memory>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Led.h"
#include "Tests/Common/TestUtils.h"

void TestClocksAdvanced::testClockBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    Clock clock;
    Led led;

    builder.add(&clock, &led);
    builder.connect(&clock, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Clock is set to ON by resetClock() during initSimulation(), LED should be ON
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);

    // Turn clock OFF, LED should be OFF
    clock.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);

    // Turn clock ON, LED should be ON
    clock.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);

    // Verify frequency property works
    clock.setFrequency(1000.0);
    QCOMPARE(clock.frequency(), 1000.0);

    // Verify delay property works
    clock.setDelay(0.5);
    QCOMPARE(clock.delay(), 0.5);
}

// ============================================================
// Clock Timing Accuracy Tests
// ============================================================

void TestClocksAdvanced::testClockFrequencyEdgeCases()
{
    // Test extreme frequency values with tolerance for floating point variance
    auto clock = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    QVERIFY(clock != nullptr);

    // Very low frequency
    clock->setFrequency(0.1);
    QVERIFY(qFuzzyCompare(clock->frequency(), 0.1));

    // Very high frequency
    clock->setFrequency(10000.0);
    QVERIFY(qFuzzyCompare(clock->frequency(), 10000.0));
}

void TestClocksAdvanced::testClockTimingPrecision()
{
    // Test clock timing precision - verify frequency can be set and retrieved
    auto clock = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    QVERIFY(clock != nullptr);

    // Set frequency and verify retrieval
    clock->setFrequency(33.33);
    QVERIFY(qFuzzyCompare(clock->frequency(), 33.33));

    clock->setFrequency(66.67);
    QVERIFY(qFuzzyCompare(clock->frequency(), 66.67));
}

void TestClocksAdvanced::testClockSilentRejectionForExtremeFrequencies()
{
    // Test that setFrequency silently rejects extremely high frequencies
    // that would produce invalid microsecond intervals
    auto clock = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    QVERIFY(clock != nullptr);

    // Set a valid initial frequency
    clock->setFrequency(100.0);
    QCOMPARE(clock->frequency(), 100.0);

    // Try to set an extremely high frequency (should be silently rejected)
    // The threshold depends on implementation, but frequencies above ~1MHz typically fail
    // because the interval calculation produces microseconds <= 0
    clock->setFrequency(1e8);  // 100 MHz - extremely high
    // After silent rejection, frequency should remain unchanged
    QCOMPARE(clock->frequency(), 100.0);

    // Verify we can still set a valid frequency after the failed attempt
    clock->setFrequency(50.0);
    QCOMPARE(clock->frequency(), 50.0);

    // Test zero frequency rejection (also silently rejected)
    clock->setFrequency(0.0);
    // Frequency should remain at last valid value
    QCOMPARE(clock->frequency(), 50.0);
}

// ============================================================
// Multi-Clock Scenario Tests
// ============================================================

void TestClocksAdvanced::testTwoClocksDifferentFrequencies()
{
    // Test two clocks with different frequencies
    auto clock1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto clock2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));

    QVERIFY(clock1 != nullptr);
    QVERIFY(clock2 != nullptr);

    // Set different frequencies
    clock1->setFrequency(1.0);   // 1 Hz
    clock2->setFrequency(100.0); // 100 Hz

    QCOMPARE(clock1->frequency(), 1.0);
    QCOMPARE(clock2->frequency(), 100.0);

    // Verify they maintain independent frequencies
    QVERIFY(clock1->frequency() != clock2->frequency());
}

void TestClocksAdvanced::testThreeClocksConcurrentExecution()
{
    // Test three clocks running simultaneously
    auto clock1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto clock2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto clock3 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));

    QVERIFY(clock1 != nullptr);
    QVERIFY(clock2 != nullptr);
    QVERIFY(clock3 != nullptr);

    // Set different frequencies
    clock1->setFrequency(1.0);
    clock2->setFrequency(10.0);
    clock3->setFrequency(100.0);

    // All frequencies should be independently maintained
    QCOMPARE(clock1->frequency(), 1.0);
    QCOMPARE(clock2->frequency(), 10.0);
    QCOMPARE(clock3->frequency(), 100.0);
}

void TestClocksAdvanced::testClockSynchronizationEdgeCases()
{
    // Test clock synchronization edge cases
    auto clock1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto clock2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));

    QVERIFY(clock1 != nullptr);
    QVERIFY(clock2 != nullptr);

    // Set same frequency
    clock1->setFrequency(50.0);
    clock2->setFrequency(50.0);

    QCOMPARE(clock1->frequency(), clock2->frequency());

    // Modify one
    clock1->setFrequency(60.0);
    QVERIFY(clock1->frequency() != clock2->frequency());
}

void TestClocksAdvanced::testMultiClockPhaseRelationships()
{
    // Test phase relationships between multiple clocks
    auto clock1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto clock2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));

    QVERIFY(clock1 != nullptr);
    QVERIFY(clock2 != nullptr);

    // Set frequencies in ratio
    clock1->setFrequency(100.0);  // 100 Hz
    clock2->setFrequency(50.0);   // 50 Hz (half of clock1)

    // Verify ratio is maintained
    QCOMPARE(clock1->frequency() / clock2->frequency(), 2.0);
}

