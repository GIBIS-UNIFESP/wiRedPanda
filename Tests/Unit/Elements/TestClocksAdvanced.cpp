// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestClocksAdvanced.h"

#include <chrono>
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
    QCOMPARE(TestUtils::inputStatus(&led), true);

    // Turn clock OFF, LED should be OFF
    clock.setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), false);

    // Turn clock ON, LED should be ON
    clock.setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), true);

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
    // Verify the clock's actual tick timing (not just that setFrequency()/frequency()
    // round-trip): the first toggle must happen at, not before or long after, the configured
    // half-period.
    auto element = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto *clock = qobject_cast<Clock *>(element.get());
    QVERIFY(clock != nullptr);

    clock->setFrequency(10.0); // 50ms half-period
    const auto t0 = std::chrono::steady_clock::now();
    clock->resetClock(t0);
    const bool initialState = clock->isOn();

    // Just before the half-period: must not have toggled yet.
    clock->updateClock(t0 + std::chrono::milliseconds(49));
    QCOMPARE(clock->isOn(), initialState);

    // Just after the half-period: must have toggled exactly once.
    clock->updateClock(t0 + std::chrono::milliseconds(51));
    QCOMPARE(clock->isOn(), !initialState);
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
    QVERIFY(!qFuzzyCompare(clock1->frequency(), clock2->frequency()));
}

void TestClocksAdvanced::testThreeClocksConcurrentExecution()
{
    // Verify three clocks with different frequencies tick independently and correctly when
    // driven together, not just that setFrequency()/frequency() round-trip. Rather than
    // hardcoding expected toggle counts (fragile to the strict-inequality boundary effect in
    // Clock::updateClock()), compare each clock's toggle count when ticked together against
    // its own toggle count when ticked in isolation over the identical timestamp sequence —
    // concurrent execution must not perturb any clock's independent timing.
    auto countToggles = [](Clock *clock, std::chrono::steady_clock::time_point t0, int durationMs) {
        clock->resetClock(t0);
        bool prev = clock->isOn();
        int toggles = 0;
        for (int ms = 1; ms <= durationMs; ++ms) {
            clock->updateClock(t0 + std::chrono::milliseconds(ms));
            if (clock->isOn() != prev) { ++toggles; prev = clock->isOn(); }
        }
        return toggles;
    };

    auto element1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto element2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto element3 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto *clock1 = qobject_cast<Clock *>(element1.get());
    auto *clock2 = qobject_cast<Clock *>(element2.get());
    auto *clock3 = qobject_cast<Clock *>(element3.get());
    QVERIFY(clock1 && clock2 && clock3);

    clock1->setFrequency(1.0);
    clock2->setFrequency(10.0);
    clock3->setFrequency(100.0);

    const auto t0 = std::chrono::steady_clock::now();
    constexpr int kDurationMs = 1000;

    // Baseline: each clock's own toggle count when ticked in isolation.
    const int isolated1 = countToggles(clock1, t0, kDurationMs);
    const int isolated2 = countToggles(clock2, t0, kDurationMs);
    const int isolated3 = countToggles(clock3, t0, kDurationMs);
    QVERIFY(isolated1 > 0);
    QVERIFY(isolated2 > isolated1);
    QVERIFY(isolated3 > isolated2);

    // Now reset and re-run all three TOGETHER, ticked at every 1ms mark simultaneously
    // (mirrors the real simulation's tick cadence) — each must toggle exactly as many times
    // as it did in isolation.
    clock1->resetClock(t0);
    clock2->resetClock(t0);
    clock3->resetClock(t0);
    bool prev1 = clock1->isOn(), prev2 = clock2->isOn(), prev3 = clock3->isOn();
    int together1 = 0, together2 = 0, together3 = 0;
    for (int ms = 1; ms <= kDurationMs; ++ms) {
        const auto t = t0 + std::chrono::milliseconds(ms);
        clock1->updateClock(t);
        clock2->updateClock(t);
        clock3->updateClock(t);
        if (clock1->isOn() != prev1) { ++together1; prev1 = clock1->isOn(); }
        if (clock2->isOn() != prev2) { ++together2; prev2 = clock2->isOn(); }
        if (clock3->isOn() != prev3) { ++together3; prev3 = clock3->isOn(); }
    }

    QCOMPARE(together1, isolated1);
    QCOMPARE(together2, isolated2);
    QCOMPARE(together3, isolated3);
}

void TestClocksAdvanced::testClockSynchronizationEdgeCases()
{
    // Verify actual synchronization behavior, not just that frequency values compare equal.
    auto element1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto element2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto *clock1 = qobject_cast<Clock *>(element1.get());
    auto *clock2 = qobject_cast<Clock *>(element2.get());
    QVERIFY(clock1 && clock2);

    clock1->setFrequency(50.0);
    clock2->setFrequency(50.0);

    const auto t0 = std::chrono::steady_clock::now();
    clock1->resetClock(t0);
    clock2->resetClock(t0);

    // Two identically-configured clocks reset at the same instant must stay in phase —
    // toggling in lockstep — across several ticks.
    for (int ms = 1; ms <= 100; ++ms) {
        const auto t = t0 + std::chrono::milliseconds(ms);
        clock1->updateClock(t);
        clock2->updateClock(t);
        QCOMPARE(clock1->isOn(), clock2->isOn());
    }

    // A clock with a phase delay must NOT stay in lockstep with an in-phase clock.
    auto element3 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto *clock3 = qobject_cast<Clock *>(element3.get());
    QVERIFY(clock3 != nullptr);
    clock3->setFrequency(50.0);
    clock3->setDelay(0.5); // half a full period — exactly one half-period out of phase
    clock3->resetClock(t0);

    bool everDiverged = false;
    for (int ms = 1; ms <= 100; ++ms) {
        const auto t = t0 + std::chrono::milliseconds(ms);
        clock1->updateClock(t);
        clock3->updateClock(t);
        if (clock1->isOn() != clock3->isOn()) {
            everDiverged = true;
        }
    }
    QVERIFY2(everDiverged, "A clock with a phase delay must diverge from an in-phase clock at some point");
}

void TestClocksAdvanced::testMultiClockPhaseRelationships()
{
    // Verify a genuine ~2:1 toggle-count ratio between two clocks in a 2:1 frequency ratio,
    // ticked together across many periods of both — not just that the frequency values
    // themselves divide to 2.0.
    auto element1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto element2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    auto *clock1 = qobject_cast<Clock *>(element1.get());
    auto *clock2 = qobject_cast<Clock *>(element2.get());
    QVERIFY(clock1 && clock2);

    clock1->setFrequency(100.0); // 5ms half-period
    clock2->setFrequency(50.0);  // 10ms half-period — half of clock1's frequency

    const auto t0 = std::chrono::steady_clock::now();
    clock1->resetClock(t0);
    clock2->resetClock(t0);

    int toggles1 = 0, toggles2 = 0;
    bool prev1 = clock1->isOn(), prev2 = clock2->isOn();
    // Many periods of both, so the strict-inequality boundary effect at the start/end of the
    // window is negligible relative to the total count.
    constexpr int kDurationMs = 2000;
    for (int ms = 1; ms <= kDurationMs; ++ms) {
        const auto t = t0 + std::chrono::milliseconds(ms);
        clock1->updateClock(t);
        clock2->updateClock(t);
        if (clock1->isOn() != prev1) { ++toggles1; prev1 = clock1->isOn(); }
        if (clock2->isOn() != prev2) { ++toggles2; prev2 = clock2->isOn(); }
    }

    QVERIFY(toggles1 > 0);
    QVERIFY(toggles2 > 0);
    const double ratio = static_cast<double>(toggles1) / static_cast<double>(toggles2);
    QVERIFY2(ratio > 1.8 && ratio < 2.2,
             qPrintable(QString("Expected ~2:1 toggle ratio, got %1:%2 (ratio %3)")
                            .arg(toggles1).arg(toggles2).arg(ratio)));
}
