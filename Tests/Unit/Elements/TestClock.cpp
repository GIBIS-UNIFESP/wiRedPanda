// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestClock.h"

#include <chrono>
#include <memory>

#include <QDataStream>
#include <QTest>

#include "App/Element/GraphicElements/Clock.h"
#include "App/IO/SerializationContext.h"
#include "Tests/Common/TestUtils.h"

using namespace std::chrono_literals;

// ============================================================================
// Constructor Tests
// ============================================================================

void TestClock::testConstructorInitialization()
{
    Clock clock;

    // Verify element type
    QCOMPARE(clock.elementType(), ElementType::Clock);

    // Verify element group is Input
    QCOMPARE(clock.elementGroup(), ElementGroup::Input);

    // Verify port configuration (0 inputs, 1 output)
    QCOMPARE(clock.inputSize(), 0);
    QCOMPARE(clock.outputSize(), 1);

    // Verify has frequency property
    QVERIFY(clock.hasFrequency());

    // Verify has delay property
    QVERIFY(clock.hasDelay());

    // Verify has label
    QVERIFY(clock.hasLabel());

    // Verify cannot rotate
    QVERIFY(!clock.isRotatable());

    // Verify can change appearance
    QVERIFY(clock.canChangeAppearance());

    // Verify initial state is OFF
    QVERIFY(!clock.isOn());

    // Verify default frequency is 1.0 Hz
    QCOMPARE(clock.frequency(), 1.0);

    // Verify default delay is 0.0
    QCOMPARE(clock.delay(), 0.0);
}

// ============================================================================
// Frequency Management Tests
// ============================================================================

void TestClock::testDefaultFrequency()
{
    Clock clock;

    // Default frequency should be 1.0 Hz
    QCOMPARE(clock.frequency(), 1.0);
}

void TestClock::testSetFrequency()
{
    Clock clock;

    // Set various valid frequencies
    clock.setFrequency(2.0);
    QCOMPARE(clock.frequency(), 2.0);

    clock.setFrequency(10.0);
    QCOMPARE(clock.frequency(), 10.0);

    clock.setFrequency(0.5);
    QCOMPARE(clock.frequency(), 0.5);
}

void TestClock::testSetFrequencyZero()
{
    Clock clock;

    // Set initial frequency
    clock.setFrequency(5.0);
    QCOMPARE(clock.frequency(), 5.0);

    // Set to zero - should be rejected, frequency remains unchanged
    clock.setFrequency(0.0);
    QCOMPARE(clock.frequency(), 5.0);
}

void TestClock::testSetFrequencyNegative()
{
    Clock clock;

    // Set initial frequency
    clock.setFrequency(3.0);
    QCOMPARE(clock.frequency(), 3.0);

    // Set to negative - should be rejected (interval = 1s / (2 * -2) = -250ms fails validation)
    clock.setFrequency(-2.0);

    // Frequency should remain unchanged at 3.0 (setter rejects invalid value)
    QCOMPARE(clock.frequency(), 3.0);
}

void TestClock::testFrequencyPersistence()
{
    Clock clock;

    // Set multiple frequencies in sequence
    clock.setFrequency(1.0);
    QCOMPARE(clock.frequency(), 1.0);

    clock.setFrequency(5.0);
    QCOMPARE(clock.frequency(), 5.0);

    clock.setFrequency(0.1);
    QCOMPARE(clock.frequency(), 0.1);

    // Verify final frequency is correct
    QCOMPARE(clock.frequency(), 0.1);
}

void TestClock::testFrequencyHighValues()
{
    Clock clock;

    // Test high frequency values
    clock.setFrequency(100.0);
    QCOMPARE(clock.frequency(), 100.0);

    clock.setFrequency(1000.0);
    QCOMPARE(clock.frequency(), 1000.0);

    clock.setFrequency(10000.0);
    QCOMPARE(clock.frequency(), 10000.0);
}

// ============================================================================
// Delay Management Tests
// ============================================================================

void TestClock::testDefaultDelay()
{
    Clock clock;

    // Default delay should be 0.0
    QCOMPARE(clock.delay(), 0.0);
}

void TestClock::testSetDelay()
{
    Clock clock;

    // Set various delays
    clock.setDelay(1.0);
    QCOMPARE(clock.delay(), 1.0);

    clock.setDelay(5.5);
    QCOMPARE(clock.delay(), 5.5);

    clock.setDelay(0.1);
    QCOMPARE(clock.delay(), 0.1);
}

void TestClock::testSetDelayNegative()
{
    Clock clock;

    // Set negative delay - should be accepted
    clock.setDelay(-1.0);
    QCOMPARE(clock.delay(), -1.0);

    // Negative delay is valid in simulation context
}

void TestClock::testDelayPersistence()
{
    Clock clock;

    // Set multiple delays in sequence
    clock.setDelay(0.5);
    QCOMPARE(clock.delay(), 0.5);

    clock.setDelay(2.0);
    QCOMPARE(clock.delay(), 2.0);

    clock.setDelay(0.0);
    QCOMPARE(clock.delay(), 0.0);

    // Verify final delay is correct
    QCOMPARE(clock.delay(), 0.0);
}

// ============================================================================
// State Management Tests
// ============================================================================

void TestClock::testInitialState()
{
    Clock clock;

    // Initial state should be OFF (set in constructor)
    QVERIFY(!clock.isOn());

    // Output port should be Inactive
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);
}

void TestClock::testSetOn()
{
    Clock clock;

    // setOn sets to true
    clock.setOn();
    QVERIFY(clock.isOn());

    // Output port should be Active
    QCOMPARE(clock.outputPort(0)->status(), Status::Active);
}

void TestClock::testSetOff()
{
    Clock clock;

    // Set to ON first
    clock.setOn();
    QVERIFY(clock.isOn());

    // setOff sets to false
    clock.setOff();
    QVERIFY(!clock.isOn());

    // Output port should be Inactive
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);
}

void TestClock::testIsOn()
{
    Clock clock;

    // Port parameter is ignored
    QVERIFY(!clock.isOn(0));
    QVERIFY(!clock.isOn(1));
    QVERIFY(!clock.isOn(-1));

    // After setOn
    clock.setOn();
    QVERIFY(clock.isOn());
    QVERIFY(clock.isOn(0));
    QVERIFY(clock.isOn(99));  // Port parameter ignored
}

void TestClock::testPortStatus()
{
    Clock clock;

    // Initial: Inactive
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);

    // After setOn
    clock.setOn();
    QCOMPARE(clock.outputPort(0)->status(), Status::Active);

    // After setOff
    clock.setOff();
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);
}

// ============================================================================
// Timing Tests
// ============================================================================

void TestClock::testResetClock()
{
    Clock clock;

    clock.setFrequency(1.0);
    clock.setDelay(0.5);

    // Get current time
    auto now = std::chrono::steady_clock::now();

    // Reset clock with delay
    clock.resetClock(now);

    // Should set to ON
    QVERIFY(clock.isOn());
}

void TestClock::testUpdateClock()
{
    Clock clock;

    clock.setFrequency(1.0);

    // Get current time
    auto now = std::chrono::steady_clock::now();

    // Reset clock to initialize timing
    clock.resetClock(now);
    QVERIFY(clock.isOn());

    // Update immediately - should not toggle (not enough time elapsed)
    clock.updateClock(now);
    QVERIFY(clock.isOn());

    // Update after 100ms - at 1 Hz half-period is 500ms, so clock should still be ON
    auto later = now + std::chrono::milliseconds(100);
    clock.updateClock(later);
    QVERIFY(clock.isOn());
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestClock::testSaveFrequencyDelay()
{
    Clock clock;

    clock.setFrequency(5.0);
    clock.setDelay(1.5);

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    clock.save(stream);

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestClock::testLoadVersionOld()
{
    // NOTE: Format Mismatch Bug
    // The current save() writes QMap<QString,QVariant> (v4.1+) but old load (v1.1-4.0)
    // expects individual fields via GraphicElement::loadOldFormat().
    // Attempting to save with current code and load with version 3.0 causes format mismatch.
    //
    // This test verifies:
    // 1. The old format load path doesn't crash
    // 2. When format is mismatched, frequency doesn't change from default

    // Create and save Clock with current code (QMap format)
    auto clock1 = std::make_unique<Clock>();
    clock1->setFrequency(2.0);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    clock1->save(saveStream);

    // Load with old version (1.1 - 4.0) - will attempt to deserialize QMap as old format fields
    auto clock2 = std::make_unique<Clock>();

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    // This doesn't crash despite format mismatch (graceful degradation)
    SerializationContext contextOld{portMap, QVersionNumber(3, 0), {}};
    clock2->load(loadStream, contextOld);

    // Due to format mismatch, frequency remains at default (setFrequency never called with valid value)
    QCOMPARE(clock2->frequency(), 1.0);
}

void TestClock::testLoadVersionNew()
{
    // Create and save Clock with both frequency and delay
    auto clock1 = std::make_unique<Clock>();
    clock1->setFrequency(10.0);
    clock1->setDelay(2.0);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    clock1->save(saveStream);

    // Load with version 4.1 (delay silently discarded for version < 4.3)
    auto clock2 = std::make_unique<Clock>();

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext contextNew{portMap, QVersionNumber(4, 1), {}};

    clock2->load(loadStream, contextNew);

    // Frequency should be loaded correctly
    QCOMPARE(clock2->frequency(), 10.0);

    // Delay should NOT be loaded (discarded for version < 4.3), remains at default
    QCOMPARE(clock2->delay(), 0.0);
}

void TestClock::testLoadVersionVeryOld()
{
    // Create Clock
    auto clock = std::make_unique<Clock>();

    // Create dummy data stream with version < 1.1
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // For version < 1.1, load should return early without reading data
    stream << quint64(1);  // Parent class data

    QDataStream readStream(data);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext contextVeryOld{portMap, QVersionNumber(1, 0), {}};

    clock->load(readStream, contextVeryOld);

    // Should not crash, frequency/delay remain at defaults
    QCOMPARE(clock->frequency(), 1.0);  // Default
    QCOMPARE(clock->delay(), 0.0);      // Default
}

// ============================================================================
// Generic Properties Tests
// ============================================================================

void TestClock::testGenericProperties()
{
    Clock clock;

    // Generic properties should return frequency in Hz format
    clock.setFrequency(1.0);
    QString props1 = clock.genericProperties();
    QVERIFY(props1.contains("Hz"));
    QVERIFY(props1.contains("1"));

    clock.setFrequency(5.0);
    QString props2 = clock.genericProperties();
    QVERIFY(props2.contains("Hz"));
    QVERIFY(props2.contains("5"));

    clock.setFrequency(0.5);
    QString props3 = clock.genericProperties();
    QVERIFY(props3.contains("Hz"));
}

