// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinExporter: renders/serializes a SignalModel to text artifacts.
 */

#pragma once

class QTextStream;
class SignalModel;

/**
 * \namespace DolphinExporter
 * \brief Model → text-artifact conversion for the beWavedDolphin truth-table export path.
 *
 * \details Originally also home to the pixmap/PNG/PDF export paths (a throwaway QTableView +
 * SignalDelegate render, then QPrinter for PDF) -- those lived in App/BeWavedDolphin/
 * BeWavedDolphin.cpp, the Widgets waveform editor, and are gone along with it.
 * QuickDolphinExporter.h/.cpp is the Quick-native replacement for image/PDF export.
 * writeTruthTableText() is the one function still genuinely shared (called from
 * QuickDolphinController.cpp) -- pure text formatting, no Widgets involved.
 */
namespace DolphinExporter {

/// Writes \a model to \a out as the truth-table text format: \a inputRowCount input rows,
/// a blank line, then the output rows, each line followed by its `: "label"`.
void writeTruthTableText(QTextStream &out, const SignalModel *model, int inputRowCount);

} // namespace DolphinExporter
