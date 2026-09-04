// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Dolphin/QuickDolphinExporter.h"

#include <algorithm>

#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QPrinter>
#include <QStandardItem>
#include <QString>

#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Common.h"

namespace {

/// Reproduces SignalDelegate::drawWaveform()'s exact band/plateau/edge-bar geometry (the same
/// fractions SignalTable.qml's Canvas delegate already duplicated, decoded from the original
/// 100x30 SVGs) via direct QPainter fillRect() calls instead of a QPainter-based QItemDelegate.
void drawWaveformCell(QPainter &painter, const QRectF &cell, WaveSegment seg, bool isInput)
{
    const QColor color = isInput ? QColor(0x75, 0x8e, 0xff) : QColor(0, 128, 0);
    QColor bandColor = color;
    bandColor.setAlpha(128);

    const double x = cell.x();
    const double y = cell.y();
    const double w = cell.width();
    const double h = cell.height();

    const bool high = (seg == WaveSegment::High) || (seg == WaveSegment::Rising);
    const double lineTop = (high ? 8.0 / 30.0 : 20.0 / 30.0) * h;
    const double lineBottom = (high ? 12.0 / 30.0 : 24.0 / 30.0) * h;
    const double thickness = (4.0 / 30.0) * h;

    painter.fillRect(QRectF(x, y + lineBottom, w, h - lineBottom), bandColor);
    painter.fillRect(QRectF(x, y + lineTop, w, thickness), color);

    if (seg == WaveSegment::Rising || seg == WaveSegment::Falling) {
        const double barTop = (8.0 / 30.0) * h;
        const double barBottom = (24.0 / 30.0) * h;
        painter.fillRect(QRectF(x, y + barTop, (4.0 / 100.0) * w, barBottom - barTop), color);
    }
}

} // namespace

namespace QuickDolphinExporter {

QImage renderToImage(const SignalModel *model, const PlotType plotType, const int cellW, const int cellH)
{
    const int rows = model->rowCount();
    const int cols = model->columnCount();

    const QFont font;
    const QFontMetrics fm(font);

    // Row-label column width: auto-sized to the widest label, mirroring QHeaderView's own
    // default content-based sizing (the original never sets an explicit vertical header width).
    int rowHeaderWidth = 0;
    for (int row = 0; row < rows; ++row) {
        const auto *header = model->verticalHeaderItem(row);
        rowHeaderWidth = std::max(rowHeaderWidth, fm.horizontalAdvance(header ? header->text() : QString()));
    }
    rowHeaderWidth += 16;
    const int colHeaderHeight = fm.height() + 8;

    const int contentW = rowHeaderWidth + cols * cellW;
    const int contentH = colHeaderHeight + rows * cellH;

    // Opaque white background, matching QTableView::grab()'s own default (unlike CanvasItem::
    // renderExportImage()'s transparent one -- this reproduces a table widget's screenshot, not
    // a circuit diagram meant to composite over other content).
    QImage image(contentW, contentH, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font);

    const QColor headerColor(240, 240, 240);
    painter.fillRect(QRect(0, 0, contentW, colHeaderHeight), headerColor);
    for (int col = 0; col < cols; ++col) {
        const QRect cellRect(rowHeaderWidth + col * cellW, 0, cellW, colHeaderHeight);
        painter.drawText(cellRect, Qt::AlignCenter, QString::number(col + 1));
    }

    for (int row = 0; row < rows; ++row) {
        const int y = colHeaderHeight + row * cellH;
        const auto *header = model->verticalHeaderItem(row);
        const QRect labelRect(0, y, rowHeaderWidth, cellH);
        painter.fillRect(labelRect, headerColor);
        painter.drawText(labelRect, Qt::AlignCenter, header ? header->text() : QString());

        // Alternating row background, matching setAlternatingRowColors(true).
        const QColor rowBg = (row % 2 == 0) ? QColor(Qt::white) : QColor(245, 245, 245);
        const bool isInput = model->isInputRow(row);

        for (int col = 0; col < cols; ++col) {
            const QRect cellRect(rowHeaderWidth + col * cellW, y, cellW, cellH);
            painter.fillRect(cellRect, rowBg);

            const int value = model->value(row, col);
            if (plotType == PlotType::Number) {
                painter.drawText(cellRect, Qt::AlignCenter, QString::number(value));
            } else {
                const bool hasPrev = col > 0;
                const int prevValue = hasPrev ? model->value(row, col - 1) : 0;
                drawWaveformCell(painter, cellRect, segmentFor(value, hasPrev, prevValue), isInput);
            }
        }
    }

    return image;
}

bool exportToPng(const SignalModel *model, const PlotType plotType, const QString &fileName)
{
    return renderToImage(model, plotType, kExportCellWidth, kExportCellHeight).save(fileName);
}

void exportToPdf(const SignalModel *model, const PlotType plotType, const QString &fileName)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Could not print this circuit to PDF.");
    }

    const QImage image = renderToImage(model, plotType, kExportCellWidth, kExportCellHeight);
    const QSize target = image.size().scaled(painter.viewport().size(), Qt::KeepAspectRatio);
    painter.drawImage(QRect(QPoint(0, 0), target), image);
    painter.end();
}

} // namespace QuickDolphinExporter
