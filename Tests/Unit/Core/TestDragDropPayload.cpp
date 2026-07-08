// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestDragDropPayload.h"

#include <QDataStream>
#include <QPoint>

#include "App/Core/DragDropPayload.h"
#include "App/Core/Enums.h"

void TestDragDropPayload::testRoundTrip()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QPoint(3, 7);
    stream << ElementType::IC;
    stream << QStringLiteral("some/ic/file.panda");
    stream << true;
    stream << QStringLiteral("my_blob");

    QDataStream readStream(&data, QIODevice::ReadOnly);
    const auto payload = readDragDropPayload(readStream);

    QCOMPARE(payload.offset, QPoint(3, 7));
    QCOMPARE(payload.type, ElementType::IC);
    QCOMPARE(payload.icFileName, QStringLiteral("some/ic/file.panda"));
    QCOMPARE(payload.isEmbedded, true);
    QCOMPARE(payload.blobName, QStringLiteral("my_blob"));
}

void TestDragDropPayload::testRejectsOversizedIcFileName()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QPoint(0, 0);
    stream << ElementType::IC;
    // Raw implausible length prefix (no real string data behind it) — this is exactly
    // what a crafted drag payload would look like; readBoundedString must reject the
    // length against the stream's actual remaining bytes instead of allocating it.
    stream << static_cast<quint32>(0x7FFFFFFF);

    QDataStream readStream(&data, QIODevice::ReadOnly);
    bool threw = false;
    try {
        readDragDropPayload(readStream);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible icFileName length must be rejected, not allocated");
}

void TestDragDropPayload::testRejectsOversizedBlobName()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QPoint(0, 0);
    stream << ElementType::IC;
    stream << QStringLiteral("real_ic.panda");
    stream << true;
    stream << static_cast<quint32>(0x7FFFFFFF);

    QDataStream readStream(&data, QIODevice::ReadOnly);
    bool threw = false;
    try {
        readDragDropPayload(readStream);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Implausible blobName length must be rejected, not allocated");
}

void TestDragDropPayload::testMissingOptionalFieldsDefaultGracefully()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QPoint(1, 2);
    stream << ElementType::And;
    stream << QStringLiteral("legacy_payload.panda");
    // Stream ends here — no isEmbedded/blobName, matching a pre-embedded-IC-support payload.

    QDataStream readStream(&data, QIODevice::ReadOnly);
    const auto payload = readDragDropPayload(readStream);

    QCOMPARE(payload.icFileName, QStringLiteral("legacy_payload.panda"));
    QCOMPARE(payload.isEmbedded, false);
    QVERIFY(payload.blobName.isEmpty());
}
