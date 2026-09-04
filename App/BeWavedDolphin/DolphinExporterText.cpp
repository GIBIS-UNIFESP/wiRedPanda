// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinExporter::writeTruthTableText()/csvText() -- pure QTextStream/QString
 * formatting, zero Widgets dependency. Split out of DolphinExporter.cpp (whose
 * renderToPixmap()/exportToPng()/exportToPdf() genuinely need QTableView/QHeaderView) so this
 * file compiles into the portable, Widgets-free SOURCES list -- wiredpanda's
 * QuickDolphinController::saveToTxt() calls writeTruthTableText() directly.
 */

#include "App/BeWavedDolphin/DolphinExporter.h"

#include <QString>
#include <QTextStream>

#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Enums.h"

namespace DolphinExporter {

/// One character per cell. The format writes cells with no separator, so every value must be
/// exactly one character wide -- a raw Status::Unknown (-1) would print as two and silently shift
/// the column count, making the row unparseable. 'x' and 'E' follow the usual HDL convention for
/// undefined and conflicting.
static QChar cellChar(const int value)
{
    if (value == static_cast<int>(Status::Unknown)) { return QLatin1Char('x'); }
    if (value == static_cast<int>(Status::Error))   { return QLatin1Char('E'); }
    return (value == 0) ? QLatin1Char('0') : QLatin1Char('1');
}

void writeTruthTableText(QTextStream &out, const SignalModel *model, const int inputRowCount)
{
    // Write input rows first, then output rows, each followed by its signal label.
    for (int row = 0; row < inputRowCount; ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << cellChar(model->value(row, col));
        }

        const auto *header = model->verticalHeaderItem(row);
        out << " : \"" << (header ? header->text() : QString()) << "\"\n";
    }

    out << "\n";

    for (int row = inputRowCount; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << cellChar(model->value(row, col));
        }

        const auto *header = model->verticalHeaderItem(row);
        out << " : \"" << (header ? header->text() : QString()) << "\"\n";
    }
}

QString csvText(const SignalModel *model)
{
    // CSV-ish format: "rows,cols," header line, then one comma-separated line per row.
    QString text;
    QTextStream out(&text);

    out << model->rowCount() << "," << model->columnCount() << ",\n";

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->value(row, col) << ",";
        }

        out << "\n";
    }

    return text;
}

} // namespace DolphinExporter
