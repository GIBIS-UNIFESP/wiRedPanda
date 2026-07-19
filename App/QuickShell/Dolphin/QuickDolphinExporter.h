// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickDolphinExporter: Quick-native replacement for DolphinExporter's pixmap rendering.
 */

#pragma once

#include "App/BeWavedDolphin/SignalDelegate.h"

class QImage;
class QString;
class SignalModel;

/**
 * \namespace QuickDolphinExporter
 * \brief Model -> image conversions for the beWavedDolphin waveform export paths (Phase 6d of
 * the Qt Quick rewrite), standing in for DolphinExporter::renderToPixmap()/exportToPng()/
 * exportToPdf().
 *
 * \details DolphinExporter::renderToPixmap() (App/BeWavedDolphin/DolphinExporter.cpp) constructs
 * a real, throwaway QTableView to rasterize -- exactly the class of gap
 * ICRenderer::generatePreviewPixmap() hit in Phase 2 (a real QWidget needs a live QApplication,
 * not just wiredpanda_quick's QGuiApplication). This namespace reimplements the same visual
 * output via direct QPainter calls over the model instead, the same substitution
 * CanvasItem::renderExportImage()/paintElementsInto() already made for CircuitExporter/
 * ICRenderer's own QGraphicsScene-based renderers. writeTruthTableText()/csvText() (pure
 * QTextStream/QString formatting, zero QWidget dependency) are reused directly from
 * DolphinExporter -- only the pixmap path needed a Quick-native rewrite.
 */
namespace QuickDolphinExporter {

/// Per-column pixel width / per-row pixel height for PNG/PDF export -- same values as
/// DolphinExporter's own kExportCellWidth/kExportCellHeight, kept private to that file
/// originally; duplicated here since this is an independent rendering path, not a shared call.
constexpr int kExportCellWidth = 50;
constexpr int kExportCellHeight = 40;

/// Renders \a model to an image: a light-grey column-number header row, a light-grey row-label
/// column (auto-sized to the widest label, mirroring QHeaderView's own default content-based
/// sizing), and \a cellW x \a cellH data cells in \a plotType (numeric text or waveform
/// graphics, reproducing SignalDelegate::drawWaveform()'s exact band/plateau/edge-bar geometry
/// -- the same fractions SignalTable.qml's per-cell Canvas delegate already duplicated for the
/// identical reason: the source lives in a QItemDelegate-coupled class this file has no
/// dependency on otherwise).
[[nodiscard]] QImage renderToImage(const SignalModel *model, PlotType plotType, int cellW, int cellH);

/// Renders \a model with \a plotType and saves it to \a fileName as a PNG. Returns \c true on
/// success. \copydoc DolphinExporter::exportToPng
bool exportToPng(const SignalModel *model, PlotType plotType, const QString &fileName);

/// Renders \a model with \a plotType and writes it to \a fileName as a landscape A4 PDF, scaled
/// to fit the page. Throws if the PDF device cannot be opened. \copydoc DolphinExporter::exportToPdf
void exportToPdf(const SignalModel *model, PlotType plotType, const QString &fileName);

} // namespace QuickDolphinExporter
