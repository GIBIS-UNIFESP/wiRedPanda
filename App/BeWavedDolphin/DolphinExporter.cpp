// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinExporter.h"

#include <QHeaderView>
#include <QPixmap>
#include <QString>
#include <QTableView>
#include <QTextStream>

#include "App/BeWavedDolphin/SignalDelegate.h"
#include "App/BeWavedDolphin/SignalModel.h"

namespace DolphinExporter {

QPixmap renderToPixmap(const SignalModel *model, const PlotType plotType, const int cellW, const int cellH)
{
    // Render through a throwaway table bound to the same model, so the live view
    // (its zoom, selection, scroll position) is never disturbed by an export.
    QTableView view;
    view.setModel(const_cast<SignalModel *>(model));
    auto *delegate = new SignalDelegate(&view);
    delegate->setPlotType(plotType);
    view.setItemDelegate(delegate);
    view.setShowGrid(false);
    view.setAlternatingRowColors(true);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view.verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view.horizontalHeader()->setDefaultSectionSize(cellW);
    view.verticalHeader()->setDefaultSectionSize(cellH);

    // Size the view to its full content so grab() captures every row/column with
    // no scrollbars and no blank padding.
    const int contentW = view.horizontalHeader()->length() + view.verticalHeader()->width();
    const int contentH = view.verticalHeader()->length() + view.horizontalHeader()->height();
    view.resize(contentW, contentH);

    return view.grab();
}

void writeTruthTableText(QTextStream &out, const SignalModel *model, const int inputRowCount)
{
    // Write input rows first, then output rows, each followed by its signal label.
    for (int row = 0; row < inputRowCount; ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->value(row, col);
        }

        out << " : \"" << model->verticalHeaderItem(row)->text() << "\"\n";
    }

    out << "\n";

    for (int row = inputRowCount; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->value(row, col);
        }

        out << " : \"" << model->verticalHeaderItem(row)->text() << "\"\n";
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
