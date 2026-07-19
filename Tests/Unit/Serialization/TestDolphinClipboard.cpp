// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinClipboard.h"

#include <limits>

#include <QApplication>
#include <QClipboard>
#include <QDataStream>
#include <QItemSelection>
#include <QMimeData>

#include "App/BeWavedDolphin/DolphinClipboard.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/IO/Serialization.h"

void TestDolphinClipboard::testPasteRoundTrip()
{
    SignalModel source(2, 3);
    source.setInputRows(2);
    source.setValue(0, 0, 1);
    source.setValue(0, 1, 0);
    source.setValue(1, 0, 0);
    source.setValue(1, 1, 1);

    QItemSelection ranges(source.index(0, 0), source.index(1, 1));

    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        DolphinClipboard::copy(source, ranges, stream);
    }

    SignalModel target(2, 3);
    target.setInputRows(2);
    QDataStream readStream(&data, QIODevice::ReadOnly);
    DolphinClipboard::paste(target, ranges, readStream);

    QCOMPARE(target.value(0, 0), 1);
    QCOMPARE(target.value(0, 1), 0);
    QCOMPARE(target.value(1, 0), 0);
    QCOMPARE(target.value(1, 1), 1);
}

void TestDolphinClipboard::testPasteTruncatesImplausibleItemCount()
{
    // A crafted payload claiming ~1.8e19 items, but only two real 24-byte entries
    // actually follow. Before the fix this spun the paste loop for that many
    // iterations instead of stopping where the real data ends.
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << std::numeric_limits<quint64>::max();
    stream << static_cast<quint64>(0) << static_cast<quint64>(0) << static_cast<quint64>(1);
    stream << static_cast<quint64>(1) << static_cast<quint64>(1) << static_cast<quint64>(1);

    SignalModel target(2, 2);
    target.setInputRows(2);
    QItemSelection ranges(target.index(0, 0), target.index(0, 0));

    QDataStream readStream(&data, QIODevice::ReadOnly);
    DolphinClipboard::paste(target, ranges, readStream);

    QCOMPARE(target.value(0, 0), 1);
    QCOMPARE(target.value(1, 1), 1);
}

void TestDolphinClipboard::testPasteWithNoDataDoesNothing()
{
    // Announces 5 items but no entry data follows at all.
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << static_cast<quint64>(5);

    SignalModel target(2, 2);
    target.setInputRows(2);
    target.setValue(0, 0, 0);
    QItemSelection ranges(target.index(0, 0), target.index(0, 0));

    QDataStream readStream(&data, QIODevice::ReadOnly);
    DolphinClipboard::paste(target, ranges, readStream);

    QCOMPARE(target.value(0, 0), 0);
}

void TestDolphinClipboard::testPasteFromClipboardAcceptsLegacyMimeType()
{
    SignalModel source(1, 3);
    source.setInputRows(1);
    source.setValue(0, 0, 1);
    source.setValue(0, 1, 0);
    source.setValue(0, 2, 1);

    QItemSelection ranges(source.index(0, 0), source.index(0, 2));

    QByteArray itemData;
    {
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writeDolphinHeader(stream);
        DolphinClipboard::copy(source, ranges, stream);
    }

    auto *mimeData = new QMimeData();
    mimeData->setData("bdolphin/copydata", itemData); // legacy MIME type, no current-type data
    QApplication::clipboard()->setMimeData(mimeData);

    SignalModel target(1, 3);
    target.setInputRows(1);
    QVERIFY(DolphinClipboard::pasteFromClipboard(target, ranges));

    QCOMPARE(target.value(0, 0), 1);
    QCOMPARE(target.value(0, 1), 0);
    QCOMPARE(target.value(0, 2), 1);
}

void TestDolphinClipboard::testPasteFromClipboardReturnsFalseWhenEmpty()
{
    QApplication::clipboard()->setText("not a waveform payload");

    SignalModel target(1, 3);
    target.setInputRows(1);
    target.setValue(0, 0, 0);
    QItemSelection ranges(target.index(0, 0), target.index(0, 0));

    QVERIFY(!DolphinClipboard::pasteFromClipboard(target, ranges));
    QCOMPARE(target.value(0, 0), 0); // unchanged
}
