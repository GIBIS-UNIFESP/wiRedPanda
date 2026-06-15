// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinExporter: renders/serializes a SignalModel to images and text artifacts.
 */

#pragma once

#include "App/BeWavedDolphin/SignalDelegate.h"

class QPixmap;
class QString;
class QTextStream;
class SignalModel;

/**
 * \namespace DolphinExporter
 * \brief Pure model → artifact conversions for the beWavedDolphin export paths.
 *
 * \details No window, dialog, or simulation state — every function takes a SignalModel
 * and produces a pixmap or text. The controller owns the file dialogs, the printer, and
 * (for the truth table) the simulation that fills the model before formatting.
 */
namespace DolphinExporter {

/// Rasterizes \a model to a pixmap via a throwaway table view (the live view is untouched),
/// using \a plotType for the cell rendering and \a cellW x \a cellH per cell.
QPixmap renderToPixmap(const SignalModel *model, PlotType plotType, int cellW, int cellH);

/// Writes \a model to \a out as the truth-table text format: \a inputRowCount input rows,
/// a blank line, then the output rows, each line followed by its `: "label"`.
void writeTruthTableText(QTextStream &out, const SignalModel *model, int inputRowCount);

/// Returns \a model encoded in the CSV-ish "rows,cols," + comma-separated values format
/// used by the terminal (print) path.
QString csvText(const SignalModel *model);

} // namespace DolphinExporter
