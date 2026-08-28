// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinExporter.h"

#include <QHeaderView>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QString>
#include <QTableView>
#include <QTextStream>

#include "App/BeWavedDolphin/SignalDelegate.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"

namespace {
constexpr int kExportCellWidth  = 50;  ///< Per-column pixel width for PNG/PDF export.
constexpr int kExportCellHeight = 40;  ///< Per-row pixel height for PNG/PDF export.
} // namespace

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

bool exportToPng(const SignalModel *model, const PlotType plotType, const QString &fileName)
{
    return renderToPixmap(model, plotType, kExportCellWidth, kExportCellHeight).save(fileName);
}

void exportToPdf(const SignalModel *model, const PlotType plotType, const QString &fileName)
{
    // Landscape A4 fits a reasonably long waveform without excessive scaling.
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QPainter painter;

    if (!painter.begin(&printer)) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Could not print this circuit to PDF.");
    }

    // Render the waveform offscreen, then scale it to fit the page preserving aspect.
    const QPixmap pixmap = renderToPixmap(model, plotType, kExportCellWidth, kExportCellHeight);
    const QSize target = pixmap.size().scaled(painter.viewport().size(), Qt::KeepAspectRatio);
    painter.drawPixmap(QRect(QPoint(0, 0), target), pixmap);
    painter.end();
}

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
