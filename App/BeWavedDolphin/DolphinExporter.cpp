// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinExporter.h"

#include <QString>
#include <QTextStream>

#include "App/BeWavedDolphin/SignalModel.h"

namespace DolphinExporter {

void writeTruthTableText(QTextStream &out, const SignalModel *model, const int inputRowCount)
{
    // Write input rows first, then output rows, each followed by its signal label.
    for (int row = 0; row < inputRowCount; ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->value(row, col);
        }

        const auto *header = model->verticalHeaderItem(row);
        out << " : \"" << (header ? header->text() : QString()) << "\"\n";
    }

    out << "\n";

    for (int row = inputRowCount; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->value(row, col);
        }

        const auto *header = model->verticalHeaderItem(row);
        out << " : \"" << (header ? header->text() : QString()) << "\"\n";
    }
}

} // namespace DolphinExporter
