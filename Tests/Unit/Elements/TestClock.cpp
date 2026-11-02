// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestClock.h"

#include <chrono>
#include <limits>
#include <memory>

#include <QDataStream>
#include <QTest>

#include "App/Element/ClockTiming.h"
#include "App/Element/Element.h"
#include "App/IO/SerializationContext.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

using namespace std::chrono_literals;

// ============================================================================
// Constructor Tests
// ============================================================================

void TestClock::testConstructorInitialization()
{
    Element clock{ElementType::Clock};

    // Verify element type
    QCOMPARE(clock.elementType(), ElementType::Clock);

    // Verify element group is Input
    QCOMPARE(clock.elementGroup(), ElementGroup::Input);

    // Verify port configuration (0 inputs, 1 output)
    QCOMPARE(clock.inputSize(), 0);
    QCOMPARE(clock.outputSize(), 1);

    // Verify has frequency property
    QVERIFY(clock.offersParam(ParamId::Frequency));

    // Verify has delay property
    QVERIFY(clock.offersParam(ParamId::Delay));

    // Verify has label
    QVERIFY(clock.hasLabel());

    // Verify cannot rotate
    QVERIFY(!clock.rotatesGraphic());

    // Verify can change appearance
    QVERIFY(clock.canChangeAppearance());

    // Verify initial state is OFF
    QVERIFY(!clock.isOn());

    // Verify default frequency is 1.0 Hz
    QCOMPARE(clock.param<double>(ParamId::Frequency), 1.0);

    // Verify default delay is 0.0
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.0);
}

// ============================================================================
// Frequency Management Tests
// ============================================================================

void TestClock::testDefaultFrequency()
{
    Element clock{ElementType::Clock};

    // Default frequency should be 1.0 Hz
    QCOMPARE(clock.param<double>(ParamId::Frequency), 1.0);
}

void TestClock::testSetFrequency()
{
    Element clock{ElementType::Clock};

    // Set various valid frequencies
    clock.setParam(ParamId::Frequency, 2.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 2.0);

    clock.setParam(ParamId::Frequency, 10.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 10.0);

    clock.setParam(ParamId::Frequency, 0.5);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 0.5);
}

void TestClock::testSetFrequencyZero()
{
    Element clock{ElementType::Clock};

    // Set initial frequency
    clock.setParam(ParamId::Frequency, 5.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 5.0);

    // Set to zero - should be rejected, frequency remains unchanged
    clock.setParam(ParamId::Frequency, 0.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 5.0);
}

void TestClock::testSetFrequencyNegative()
{
    Element clock{ElementType::Clock};

    // Set initial frequency
    clock.setParam(ParamId::Frequency, 3.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 3.0);

    // Set to negative - should be rejected (interval = 1s / (2 * -2) = -250ms fails validation)
    clock.setParam(ParamId::Frequency, -2.0);

    // Frequency should remain unchanged at 3.0 (setter rejects invalid value)
    QCOMPARE(clock.param<double>(ParamId::Frequency), 3.0);
}

void TestClock::testFrequencyPersistence()
{
    Element clock{ElementType::Clock};

    // Set multiple frequencies in sequence
    clock.setParam(ParamId::Frequency, 1.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 1.0);

    clock.setParam(ParamId::Frequency, 5.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 5.0);

    clock.setParam(ParamId::Frequency, 0.1);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 0.1);

    // Verify final frequency is correct
    QCOMPARE(clock.param<double>(ParamId::Frequency), 0.1);
}

void TestClock::testFrequencyHighValues()
{
    Element clock{ElementType::Clock};

    // Test high frequency values
    clock.setParam(ParamId::Frequency, 100.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 100.0);

    clock.setParam(ParamId::Frequency, 1000.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 1000.0);

    clock.setParam(ParamId::Frequency, 10000.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 10000.0);
}

// ============================================================================
// Delay Management Tests
// ============================================================================

void TestClock::testDefaultDelay()
{
    Element clock{ElementType::Clock};

    // Default delay should be 0.0
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.0);
}

void TestClock::testSetDelay()
{
    Element clock{ElementType::Clock};

    // In-range values pass through verbatim.
    clock.setParam(ParamId::Delay, 1.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), 1.0);

    clock.setParam(ParamId::Delay, 0.1);
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.1);

    // Out-of-range finite values clamp to the documented [-1, 1] range.
    clock.setParam(ParamId::Delay, 5.5);
    QCOMPARE(clock.param<double>(ParamId::Delay), 1.0);

    clock.setParam(ParamId::Delay, -3.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), -1.0);

    // Non-finite values are rejected and leave m_delay unchanged.
    clock.setParam(ParamId::Delay, 0.25);
    clock.setParam(ParamId::Delay, std::numeric_limits<double>::infinity());
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.25);

    clock.setParam(ParamId::Delay, std::numeric_limits<double>::quiet_NaN());
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.25);
}

void TestClock::testSetDelayNegative()
{
    Element clock{ElementType::Clock};

    // Set negative delay - should be accepted
    clock.setParam(ParamId::Delay, -1.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), -1.0);

    // Negative delay is valid in simulation context
}

void TestClock::testDelayPersistence()
{
    Element clock{ElementType::Clock};

    // Set multiple delays in sequence
    clock.setParam(ParamId::Delay, 0.5);
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.5);

    clock.setParam(ParamId::Delay, 1.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), 1.0);

    clock.setParam(ParamId::Delay, 0.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.0);

    // Verify final delay is correct
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.0);
}

// ============================================================================
// State Management Tests
// ============================================================================

void TestClock::testInitialState()
{
    Element clock{ElementType::Clock};

    // Initial state should be OFF (set in constructor)
    QVERIFY(!clock.isOn());

    // Output port should be Inactive
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);
}

void TestClock::testSetOn()
{
    Element clock{ElementType::Clock};

    // setOn sets to true
    clock.setOn(true);
    QVERIFY(clock.isOn());

    // Output port should be Active
    QCOMPARE(clock.outputPort(0)->status(), Status::Active);
}

void TestClock::testSetOff()
{
    Element clock{ElementType::Clock};

    // Set to ON first
    clock.setOn(true);
    QVERIFY(clock.isOn());

    // A low level sets it off
    clock.setOn(false);
    QVERIFY(!clock.isOn());

    // Output port should be Inactive
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);
}

void TestClock::testIsOn()
{
    Element clock{ElementType::Clock};

    // Port parameter is ignored
    QVERIFY(!clock.isOn(0));
    QVERIFY(!clock.isOn(1));
    QVERIFY(!clock.isOn(-1));

    // After setOn
    clock.setOn(true);
    QVERIFY(clock.isOn());
    QVERIFY(clock.isOn(0));
    QVERIFY(clock.isOn(99));  // Port parameter ignored
}

void TestClock::testPortStatus()
{
    Element clock{ElementType::Clock};

    // Initial: Inactive
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);

    // After setOn
    clock.setOn(true);
    QCOMPARE(clock.outputPort(0)->status(), Status::Active);

    // After a low level
    clock.setOn(false);
    QCOMPARE(clock.outputPort(0)->status(), Status::Inactive);
}

// ============================================================================
// Timing Tests
// ============================================================================

void TestClock::testResetClock()
{
    Element clock{ElementType::Clock};

    clock.setParam(ParamId::Frequency, 1.0);
    clock.setParam(ParamId::Delay, 0.5);

    // Get current time
    auto now = std::chrono::steady_clock::now();

    // Reset clock with delay
    ClockTiming::reset(clock, now);

    // Should set to ON
    QVERIFY(clock.isOn());
}

void TestClock::testUpdateClock()
{
    Element clock{ElementType::Clock};

    clock.setParam(ParamId::Frequency, 1.0);

    // Get current time
    auto now = std::chrono::steady_clock::now();

    // Reset clock to initialize timing
    ClockTiming::reset(clock, now);
    QVERIFY(clock.isOn());

    // Update immediately - should not toggle (not enough time elapsed)
    ClockTiming::update(clock, now);
    QVERIFY(clock.isOn());

    // Update after 100ms - at 1 Hz half-period is 500ms, so clock should still be ON
    auto later = now + std::chrono::milliseconds(100);
    ClockTiming::update(clock, later);
    QVERIFY(clock.isOn());
}

void TestClock::testALockedClockHoldsItsLevel()
{
    // At 10 Hz each level lasts 50 ms: a locked clock holds its level past it, and toggles once unlocked
    Element clock{ElementType::Clock};
    clock.setParam(ParamId::Frequency, 10.0);
    const auto t0 = std::chrono::steady_clock::now();
    ClockTiming::reset(clock, t0);
    QVERIFY(clock.isOn());

    clock.setLocked(true);
    ClockTiming::update(clock, t0 + std::chrono::milliseconds(60));
    QVERIFY(clock.isOn());

    clock.setLocked(false);
    ClockTiming::update(clock, t0 + std::chrono::milliseconds(60));
    QVERIFY(!clock.isOn());
}

void TestClock::testADelayShiftsTheFirstToggle()
{
    // At 10 Hz the period is 100 ms and the first toggle comes 50 ms after a reset: a delay of a quarter period puts it
    // 25 ms later, and an advance of a quarter 25 ms earlier
    const auto t0 = std::chrono::steady_clock::now();
    const auto levelAt = [t0](const double delay, const int ms) {
        Element clock{ElementType::Clock};
        clock.setParam(ParamId::Frequency, 10.0);
        clock.setParam(ParamId::Delay, delay);
        ClockTiming::reset(clock, t0);
        ClockTiming::update(clock, t0 + std::chrono::milliseconds(ms));
        return clock.isOn();
    };
    QVERIFY(levelAt(0.25, 70));
    QVERIFY(!levelAt(0.25, 80));
    QVERIFY(levelAt(-0.25, 20));
    QVERIFY(!levelAt(-0.25, 30));
}

void TestClock::testUpdateClockLargeBacklogResyncsInsteadOfBursting()
{
    // Regression: a large elapsed gap (e.g. the OS suspended the process while the
    // simulation's QTimer stayed "active") used to be replayed as one toggle per subsequent
    // ClockTiming::update() call until caught up — for a long-enough gap, thousands of rapid toggles.
    // ClockTiming::update() must instead resync immediately, on the very first call after the gap,
    // exactly like Simulation::start()'s explicit ClockTiming::reset() already does for the
    // stop/start case.
    Element clock{ElementType::Clock};
    clock.setParam(ParamId::Frequency, 1.0); // 500ms half-period

    const auto t0 = std::chrono::steady_clock::now();
    ClockTiming::reset(clock, t0);
    QVERIFY(clock.isOn());

    // Simulate a huge backlog: far beyond the resync threshold.
    const auto hugeGap = t0 + std::chrono::hours(1);
    ClockTiming::update(clock, hugeGap);
    const bool stateAfterFirstCall = clock.isOn();

    // If ClockTiming::update() resynced (the fix), its new internal time reference is now close to
    // hugeGap, so a second call at the SAME timestamp is a no-op. If it instead single-stepped
    // (the pre-fix behavior), it would still be far behind hugeGap and would toggle again.
    ClockTiming::update(clock, hugeGap);
    QCOMPARE(clock.isOn(), stateAfterFirstCall);
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestClock::testSaveFrequencyDelay()
{
    Element clock{ElementType::Clock};

    clock.setParam(ParamId::Frequency, 5.0);
    clock.setParam(ParamId::Delay, 1.5);

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    clock.save(stream, {.purpose = SerializationPurpose::PortableFile});

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestClock::testLoadVersionOld()
{
    // NOTE: Format Mismatch Bug
    // The current save() writes QMap<QString,QVariant> (v4.1+) but old load (v1.1-4.0)
    // expects individual fields via Element::loadOldFormat().
    // Attempting to save with current code and load with version 3.0 causes format mismatch.
    //
    // This test verifies:
    // 1. The old format load path doesn't crash
    // 2. When format is mismatched, frequency doesn't change from default

    // Create and save Clock with current code (QMap format)
    auto clock1 = std::make_unique<Element>(ElementType::Clock);
    clock1->setParam(ParamId::Frequency, 2.0);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    clock1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load with old version (1.1 - 4.0) — format mismatch: save wrote QMap but
    // loadOldFormat reads positional fields.  readBoundedString rejects the map
    // count bytes as an oversized string length → PANDACEPTION is the expected result.
    auto clock2 = std::make_unique<Element>(ElementType::Clock);

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext contextOld = {portMap, QVersionNumber(3, 0), SerializationPurpose::PortableFile, {}};
    bool threw = false;
    try {
        clock2->load(loadStream, contextOld);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw); // format mismatch now throws rather than silently reading garbage
}

void TestClock::testLoadVersionNew()
{
    // Create and save Clock with both frequency and delay
    auto clock1 = std::make_unique<Element>(ElementType::Clock);
    clock1->setParam(ParamId::Frequency, 10.0);
    clock1->setParam(ParamId::Delay, 2.0);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    clock1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load with version 4.1 (delay silently discarded for version < 4.3)
    auto clock2 = std::make_unique<Element>(ElementType::Clock);

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext contextNew = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, {}};

    clock2->load(loadStream, contextNew);

    // Frequency should be loaded correctly
    QCOMPARE(clock2->param<double>(ParamId::Frequency), 10.0);

    // Delay should NOT be loaded (discarded for version < 4.3), remains at default
    QCOMPARE(clock2->param<double>(ParamId::Delay), 0.0);
}

void TestClock::testLoadVersionVeryOld()
{
    // Create Clock
    auto clock = std::make_unique<Element>(ElementType::Clock);

    // Create dummy data stream with version < 1.1
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // For version < 1.1, load should return early without reading data
    stream << quint64(1);  // Parent class data

    QDataStream readStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext contextVeryOld = {portMap, QVersionNumber(1, 0), SerializationPurpose::PortableFile, {}};

    clock->load(readStream, contextVeryOld);

    // Should not crash, frequency/delay remain at defaults
    QCOMPARE(clock->param<double>(ParamId::Frequency), 1.0);  // Default
    QCOMPARE(clock->param<double>(ParamId::Delay), 0.0);      // Default
}

void TestClock::testLoadWithNonNumericDelayFallsBackToZero()
{
    // Force map.value("delay").toDouble(&ok) to fail by rewriting the saved "delay" entry
    // to a non-numeric QVariant, exercising load()'s "ok" fallback (delayValue = 0.0).
    auto clock1 = std::make_unique<Element>(ElementType::Clock);
    clock1->setParam(ParamId::Frequency, 10.0);
    clock1->setParam(ParamId::Delay, 0.5);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    clock1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    QMap<QString, QVariant> propsMap;
    QList<QMap<QString, QVariant>> inputPorts, outputPorts, appearances;
    QMap<QString, QVariant> clockMap;
    {
        QDataStream readStream(data);
        readStream >> propsMap >> inputPorts >> outputPorts >> appearances >> clockMap;
    }
    QVERIFY(clockMap.contains("delay"));
    clockMap["delay"] = QVariant(QStringList{"not", "a", "number"}); // toDouble() fails on this

    QByteArray rewritten;
    {
        QDataStream writeStream(&rewritten, QIODevice::WriteOnly);
        writeStream << propsMap << inputPorts << outputPorts << appearances << clockMap;
    }

    auto clock2 = std::make_unique<Element>(ElementType::Clock);
    QDataStream loadStream(rewritten);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_4_3, SerializationPurpose::PortableFile};
    clock2->load(loadStream, context);

    QCOMPARE(clock2->param<double>(ParamId::Frequency), 10.0); // unaffected sibling field still loads correctly
    QCOMPARE(clock2->param<double>(ParamId::Delay), 0.0);      // falls back to 0.0 rather than keeping stale/garbage state
}
