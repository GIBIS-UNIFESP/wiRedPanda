// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QTest>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElements/InputGND.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Nodes/QNEConnection.h"

// Shared test implementations for AudioBox and Buzzer.
// Both classes expose refresh(), isPlaying(), mute(), isMuted(), and inputPort(),
// but don't inherit these from a common base, so template helpers are used.
namespace AudioElementTestHelpers {

template<typename T>
inline void testCopyConstructor(const T &original, const T &copy)
{
    QCOMPARE(copy.elementType(), original.elementType());
    QCOMPARE(copy.inputSize(), original.inputSize());
    QCOMPARE(copy.outputSize(), original.outputSize());
}

template<typename T>
inline void testPlayStopTransition(T &element)
{
    InputVcc vcc;
    auto connection = std::make_unique<QNEConnection>();
    connection->setStartPort(vcc.outputPort());
    connection->setEndPort(element.inputPort(0));

    element.refresh();
    QVERIFY(element.isPlaying());

    InputGnd gnd;
    connection->setStartPort(gnd.outputPort());

    element.refresh();
    QVERIFY(!element.isPlaying());
    connection.reset();  // Must destroy before gnd (created after connection, destroyed first otherwise)
}

template<typename T>
inline void testPlayIdempotency(T &element)
{
    InputVcc vcc;
    auto connection = std::make_unique<QNEConnection>();
    connection->setStartPort(vcc.outputPort());
    connection->setEndPort(element.inputPort(0));

    element.refresh();
    QVERIFY(element.isPlaying());

    element.refresh();
    QVERIFY(element.isPlaying());
}

template<typename T>
inline void testStopIdempotency(T &element)
{
    element.inputPort(0)->setStatus(Enums::Status::Inactive);
    element.refresh();
    QVERIFY(!element.isPlaying());

    element.refresh();
    QVERIFY(!element.isPlaying());
}

template<typename T>
inline void testRefreshWithActiveInput(T &element)
{
    InputVcc vcc;
    auto connection = std::make_unique<QNEConnection>();
    connection->setStartPort(vcc.outputPort());
    connection->setEndPort(element.inputPort(0));

    element.refresh();
    QVERIFY(element.isPlaying());
}

template<typename T>
inline void testRefreshWithInactiveInput(T &element)
{
    element.inputPort(0)->setStatus(Enums::Status::Inactive);
    element.refresh();
    QVERIFY(!element.isPlaying());
}

template<typename T>
inline void testMute(T &element)
{
    element.mute(true);
    QVERIFY(element.isMuted());
}

template<typename T>
inline void testUnmute(T &element)
{
    element.mute(true);
    QVERIFY(element.isMuted());

    element.mute(false);
    QVERIFY(!element.isMuted());
}
} // namespace AudioElementTestHelpers
