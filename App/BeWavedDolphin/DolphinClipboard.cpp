// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinClipboard.h"

#include <QDataStream>
#include <QItemSelection>

#include "App/BeWavedDolphin/SignalModel.h"

namespace DolphinClipboard {

int firstColumn(const SignalModel &model, const QItemSelection &ranges)
{
    int firstCol = model.columnCount() - 1;

    for (const auto &range : ranges) {
        if (range.left() < firstCol) {
            firstCol = range.left();
        }
    }

    return firstCol;
}

int firstRow(const SignalModel &model, const QItemSelection &ranges)
{
    int row = model.rowCount() - 1;

    for (const auto &range : ranges) {
        if (range.top() < row) {
            row = range.top();
        }
    }

    return row;
}

void copy(const SignalModel &model, const QItemSelection &ranges, QDataStream &stream)
{
    const int anchorRow = firstRow(model, ranges);
    const int anchorCol = firstColumn(model, ranges);
    const auto itemList = ranges.indexes();
    stream << static_cast<qint64>(itemList.size());

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

    for (int i = 0; i < static_cast<int>(itemListSize); ++i) {
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

} // namespace DolphinClipboard
