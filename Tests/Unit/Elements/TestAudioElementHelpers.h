// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDataStream>
#include <QKeySequence>
#include <QPointF>
#include <QTest>

#include "App/IO/SerializationContext.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"

// Shared test implementations for AudioBox and Buzzer, as templates over the element's type.
namespace AudioElementTestHelpers {

template<typename T>
inline void testCopyConstructor(const T &original, const T &copy)
{
    QCOMPARE(copy.elementType(), original.elementType());
    QCOMPARE(copy.inputSize(), original.inputSize());
    QCOMPARE(copy.outputSize(), original.outputSize());
}

/// Hand-builds a pre-2.4 (audio-not-yet-invented) old-format element stream and loads it into
/// \a element, exercising the "!hasAudio(version)" early return that no real fixture can reach:
/// audio elements (AudioBox/Buzzer) were only introduced at V_2_4, so no genuine .panda file
/// predating that version can ever contain one to load from -- this is the only way to reach
/// that guard clause at all. Layout mirrors ElementSerializer::loadOldFormat() exactly,
/// with 0 ports/appearances to sidestep needing valid port-name/appearance-index bookkeeping.
/// The caller is left to assert its own type-specific post-condition (audio()/frequency()
/// staying at whatever it was before, since load() must return before touching it).
template<typename T>
inline void testLoadBeforeAudioExisted(T &element)
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << QPointF(10.0, 20.0);   // loadPos
    writeStream << 0.0;                   // loadRotation (qreal)
    writeStream << QString("old label");  // loadLabel (hasLabels since V_1_2)
    writeStream << quint64(1) << quint64(1) << quint64(1) << quint64(1); // loadPortsSize (V_1_3)
    writeStream << QKeySequence();        // loadTrigger (V_1_9)
    writeStream << quint64(0);            // loadInputPorts: inputSize = 0
    writeStream << quint64(0);            // loadOutputPorts: outputSize = 0
    // hasAppearanceNames() is V_2_7 -- false below V_2_4, so nothing further is written.

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_1_9, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    element.load(readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
}
} // namespace AudioElementTestHelpers
