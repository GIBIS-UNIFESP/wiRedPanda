// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinClipboard.h"

#include <algorithm>

#include <QApplication>
#include <QClipboard>
#include <QDataStream>
#include <QItemSelection>
#include <QMimeData>

#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Common.h"
#include "App/IO/Serialization.h"

namespace {
/// MIME type carrying serialized beWavedDolphin waveform cells on the clipboard.
constexpr auto kWaveformMimeType = "application/x-bewaveddolphin-waveform";
/// Legacy MIME type from older versions, still accepted on paste for compatibility.
constexpr auto kLegacyMimeType = "bdolphin/copydata";

/// Returns the minimum of \a extract(range) across \a ranges, or \a fallback if empty.
template <typename Extract>
int minAcrossRanges(int fallback, const QItemSelection &ranges, Extract extract)
{
    int result = fallback;
    for (const auto &range : ranges) {
        result = (std::min)(result, extract(range));
    }
    return result;
}
} // namespace

namespace DolphinClipboard {

int firstColumn(const SignalModel &model, const QItemSelection &ranges)
{
    return minAcrossRanges(model.columnCount() - 1, ranges, [](const auto &r) { return r.left(); });
}

int firstRow(const SignalModel &model, const QItemSelection &ranges)
{
    return minAcrossRanges(model.rowCount() - 1, ranges, [](const auto &r) { return r.top(); });
}

void copy(const SignalModel &model, const QItemSelection &ranges, QDataStream &stream)
{
    const int anchorRow = firstRow(model, ranges);
    const int anchorCol = firstColumn(model, ranges);
    const auto itemList = ranges.indexes();
    const qint64 sz = itemList.size();
    stream << sz;

    for (const auto &item : itemList) {
        const int value = model.value(item.row(), item.column());
        // Store offsets relative to the selection origin so paste can re-anchor
        // the data at any target cell regardless of absolute position
        stream << static_cast<qint64>(item.row() - anchorRow);
        stream << static_cast<qint64>(item.column() - anchorCol);
        stream << static_cast<qint64>(value);
    }
}

void paste(SignalModel &model, const QItemSelection &ranges, QDataStream &stream)
{
    const int anchorCol = firstColumn(model, ranges);
    const int anchorRow = firstRow(model, ranges);
    quint64 itemListSize; stream >> itemListSize;

    // The clipboard is not a trusted source (any process can set MIME data ahead of a
    // Ctrl+V) — bound the announced item count by what the stream can actually hold
    // before looping, the same way DolphinSerializer::loadBinary bounds its row count,
    // so a crafted/corrupt payload can't spin this loop for eons instead of failing fast.
    constexpr qint64 kBytesPerItem = 3 * static_cast<qint64>(sizeof(quint64));
    const qint64 available = stream.device() ? stream.device()->bytesAvailable() : 0;
    const quint64 maxItems = available > 0 ? static_cast<quint64>(available / kBytesPerItem) : 0;
    if (itemListSize > maxItems) {
        qCWarning(zero) << "DolphinClipboard: truncating paste from" << itemListSize << "items to" << maxItems << "(insufficient stream data)";
        itemListSize = maxItems;
    }

    SignalModel::BulkEditGuard guard(model);
    for (quint64 i = 0; i < itemListSize; ++i) {
        quint64 row;   stream >> row;
        quint64 col;   stream >> col;
        quint64 value; stream >> value;
        // Re-anchor the stored relative offsets to the paste-target cell
        const int newRow = static_cast<int>(static_cast<quint64>(anchorRow) + row);
        const int newCol = static_cast<int>(static_cast<quint64>(anchorCol) + col);

        // Silently drop cells that land outside the input rows or past the simulation
        // length; output rows are never editable
        if ((newRow < model.inputRows()) && (newCol < model.columnCount())) {
            model.setValue(newRow, newCol, static_cast<int>(value));
        }
    }
}

void copyToClipboard(const SignalModel &model, const QItemSelection &ranges)
{
    // Serialise using a versioned header so paste can verify format compatibility.
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writeDolphinHeader(stream);
    copy(model, ranges, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData(kWaveformMimeType, itemData);
    QApplication::clipboard()->setMimeData(mimeData);
}

bool pasteFromClipboard(SignalModel &model, const QItemSelection &ranges)
{
    const auto *mimeData = QApplication::clipboard()->mimeData();
    QByteArray itemData;

    // Support both the legacy MIME type and the current one, so files or clipboard
    // data from older versions of the app can still be pasted.
    if (mimeData->hasFormat(kLegacyMimeType)) {
        itemData = mimeData->data(kLegacyMimeType);
    }

    if (mimeData->hasFormat(kWaveformMimeType)) {
        itemData = mimeData->data(kWaveformMimeType);
    }

    if (itemData.isEmpty()) {
        return false;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readDolphinHeader(stream);
    paste(model, ranges, stream);
    return true;
}

} // namespace DolphinClipboard
