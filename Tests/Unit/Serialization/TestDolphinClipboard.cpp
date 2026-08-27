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
#include "App/Core/Enums.h"
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

void TestDolphinClipboard::testPasteClampsInputRowValues()
{
    SignalModel model(4, 6);
    model.setInputRows(2);                 // rows 0-1 inputs, rows 2-3 outputs
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 6; ++c) {
            model.setValue(r, c, 0);
        }
    }

    // A swept Unknown and a swept Error, exactly as WaveformSimulator::sweep() writes them.
    model.setValue(3, 1, static_cast<int>(Status::Unknown));
    model.setValue(3, 2, static_cast<int>(Status::Error));
    QCOMPARE(model.value(3, 1), -1);
    QCOMPARE(model.value(3, 2), 2);

    // Copy those two output cells and paste them onto an input row.
    const QItemSelection copyRange(model.index(3, 1), model.index(3, 2));
    DolphinClipboard::copyToClipboard(model, copyRange);
    const QItemSelection pasteRange(model.index(0, 0), model.index(0, 0));
    QVERIFY(DolphinClipboard::pasteFromClipboard(model, pasteRange));

    for (int col = 0; col <= 1; ++col) {
        const int landed = model.value(0, col);
        QVERIFY2(landed == 0 || landed == 1,
                 qPrintable(QString("input cell (0,%1) holds %2; input rows must be two-state, "
                                    "or the sweep's `value != 0` reads a non-definite cell HIGH")
                                .arg(col).arg(landed)));
    }

    // Output rows must still accept four-state, or the waveform loses its Unknown/Error display.
    model.setValue(2, 0, static_cast<int>(Status::Unknown));
    QCOMPARE(model.value(2, 0), -1);
}
