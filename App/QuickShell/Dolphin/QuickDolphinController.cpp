// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Dolphin/QuickDolphinController.h"

#include <algorithm>
#include <cmath>

#include <QClipboard>
#include <QDir>
#include <QGuiApplication>
#include <QItemSelection>
#include <QTextStream>

#include "App/BeWavedDolphin/DolphinClipboard.h"
#include "App/BeWavedDolphin/DolphinCommands.h"
#include "App/BeWavedDolphin/DolphinEdits.h"
#include "App/BeWavedDolphin/DolphinExporter.h"
#include "App/BeWavedDolphin/DolphinFile.h"
#include "App/BeWavedDolphin/DolphinHost.h"
#include "App/BeWavedDolphin/DolphinModelBuilder.h"
#include "App/BeWavedDolphin/SignalDelegate.h"
#include "App/BeWavedDolphin/WaveformSimulator.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Dolphin/QuickDolphinExporter.h"
#include "App/QuickShell/Dolphin/QuickDolphinZoom.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/FileDialogProvider.h"

namespace {
/// Mirrors BewavedDolphin::snapshotCells(): pairs each index's (row, col) with its current
/// value, for constructing a SetCellsCommand right before an edit that will change them.
std::pair<QList<QPair<int, int>>, QList<int>> snapshotCells(const SignalModel &model, const QModelIndexList &indexes)
{
    QList<QPair<int, int>> cells;
    QList<int> oldValues;
    cells.reserve(indexes.size());
    oldValues.reserve(indexes.size());
    for (const auto &idx : indexes) {
        cells.append({idx.row(), idx.column()});
        oldValues.append(model.value(idx.row(), idx.column()));
    }
    return {cells, oldValues};
}

/// Mirrors BewavedDolphin::allCellIndexes(): every (row, col) index in [0, rows) x [0, cols).
QModelIndexList allCellIndexes(const SignalModel &model, int rows, int cols)
{
    QModelIndexList indexes;
    indexes.reserve(rows * cols);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            indexes.append(model.index(row, col));
        }
    }
    return indexes;
}
} // namespace

QuickDolphinController::QuickDolphinController(QObject *parent)
    : QObject(parent)
    , m_zoom(std::make_unique<QuickDolphinZoom>())
{
    // Mirrors QuickAppController's own reemit-on-any-undo-signal pattern, simplified since this
    // controller owns one QUndoStack directly rather than one per tab.
    const auto reemit = [this] { emit undoRedoStateChanged(); };
    connect(&m_undoStack, &QUndoStack::canUndoChanged, this, reemit);
    connect(&m_undoStack, &QUndoStack::canRedoChanged, this, reemit);
    connect(&m_undoStack, &QUndoStack::undoTextChanged, this, reemit);
    connect(&m_undoStack, &QUndoStack::redoTextChanged, this, reemit);

    // Mirrors QuickAppController's identical status-message-timer setup.
    m_statusMessageTimer.setSingleShot(true);
    connect(&m_statusMessageTimer, &QTimer::timeout, this, [this] {
        m_statusMessage.clear();
        emit statusMessageChanged();
    });
}

QuickDolphinController::~QuickDolphinController() = default;

qreal QuickDolphinController::columnWidth() const
{
    return m_zoom->columnWidth();
}

qreal QuickDolphinController::rowHeight() const
{
    return m_zoom->rowHeight();
}

qreal QuickDolphinController::fontScale() const
{
    return m_zoom->fontScale();
}

bool QuickDolphinController::canZoomIn() const
{
    return m_zoom->canZoomIn();
}

bool QuickDolphinController::canZoomOut() const
{
    return m_zoom->canZoomOut();
}

void QuickDolphinController::createWaveform(CanvasItem *canvas, const QString &fileName)
{
    m_canvas = canvas;
    m_simulation = canvas->simulation();
    // Construct the sweep driver before loadNewTable(), whose initial clear triggers run() --
    // mirrors BewavedDolphin::prepare()'s identical ordering comment.
    m_simDriver = std::make_unique<WaveformSimulator>(m_simulation);

    const auto tableSignals = DolphinModelBuilder::collect(canvas->elements());
    m_inputs = tableSignals.inputs;
    m_outputs = tableSignals.outputs;
    m_inputPorts = tableSignals.inputPorts;

    loadNewTable(tableSignals.inputLabels, tableSignals.outputLabels);

    if (!fileName.isEmpty()) {
        // Try the stored path as-is first (handles an absolute path); if that doesn't resolve,
        // fall back to just the filename relative to the host's working directory so relative
        // paths stored inside .panda files still resolve. Mirrors BewavedDolphin::
        // createWaveform(const QString &)'s identical fallback exactly.
        QFileInfo fileInfo(fileName);
        if ((fileInfo.isRelative() || !fileInfo.exists()) && m_host) {
            fileInfo.setFile(m_host->currentDir(), QFileInfo(fileName).fileName());
        }

        if (fileInfo.exists()) {
            loadFile(fileInfo.absoluteFilePath());
        } else {
            showStatusMessage(tr("File \"%1\" does not exist!").arg(fileName), 4000);
        }
    }

    // loadNewTable()'s initial clear (and, on the file-load branch, applyWaveformData() via
    // loadFile()) may have pushed/undone entries or flipped m_edited as part of setting up the
    // document -- neither constitutes a user edit, so both reset clean from here, mirroring
    // BewavedDolphin::createWaveform(const QString &)'s own tail exactly.
    m_edited = false;
    m_undoStack.clear();
}

void QuickDolphinController::loadNewTable(const QStringList &inputLabels, const QStringList &outputLabels)
{
    // Unlike BewavedDolphin (one window, one shot, WA_DeleteOnClose), this controller can be
    // reused across multiple createWaveform() calls -- drop any previous model/selection, undo
    // history, and file association first, so a new circuit's waveform never inherits a stale
    // one's state.
    delete m_model;
    delete m_selectionModel;
    m_undoStack.clear();
    m_currentFile = QFileInfo();
    emit currentFileChanged();

    m_model = new SignalModel(static_cast<int>(inputLabels.size() + outputLabels.size()), m_length, this);
    m_model->setVerticalHeaderLabels(inputLabels + outputLabels);
    m_model->setInputRows(static_cast<int>(inputLabels.size()));

    m_selectionModel = new QItemSelectionModel(m_model, this);
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &QuickDolphinController::hasSelectionChanged);

    // Initialize all input cells to 0 and compute the first output sweep -- mirrors
    // BewavedDolphin::loadNewTable()'s own on_actionClear_triggered() call, minus the
    // undo-command push: the original's very first Clear does land on its undo stack (a
    // real, harmless quirk -- undoing it just restores the same all-zero freshly-constructed
    // values), not worth replicating exactly for a no-observable-difference initial state.
    DolphinEdits::clearInputs(*m_model, m_inputPorts);
    run();

    emit modelChanged();
}

void QuickDolphinController::run()
{
    if (!m_simDriver || !m_model || !m_canvas) {
        return;
    }

    // An input/output element this waveform was built from may have been deleted from the live
    // canvas since createWaveform() ran -- possible for the first time now that this window is
    // actually reachable from the live app (Phase 6e): the main canvas and this window coexist,
    // and nothing blocks editing the former while the latter is open (mirrors BewavedDolphin::
    // run()'s identical elementsStillLive() guard and its own doc comment on why).
    if (!elementsStillLive()) {
        return;
    }

    SignalModel::BulkEditGuard guard(*m_model);
    const QVector<Status> oldInputValues = WaveformSimulator::captureInputs(m_inputs, m_inputPorts);

    m_simDriver->sweep(
        m_inputs, m_outputs, m_canvas->elements(), m_inputPorts, m_model->columnCount(),
        [this](int row, int col) { return m_model->value(row, col) != 0; },
        [this](int row, int col, int value) { m_model->setValue(row, col, value); });

    WaveformSimulator::restoreInputs(m_inputs, oldInputValues);
}

void QuickDolphinController::setCellValue(int row, int col, int value)
{
    if (!m_model) {
        return;
    }
    m_model->setValue(row, col, value);
    run();
}

void QuickDolphinController::showNumbers()
{
    if (m_numberMode) {
        return;
    }
    m_numberMode = true;
    emit numberModeChanged();
}

void QuickDolphinController::showWaveforms()
{
    if (!m_numberMode) {
        return;
    }
    m_numberMode = false;
    emit numberModeChanged();
}

void QuickDolphinController::zoomIn()
{
    m_zoom->zoomIn();
    emit zoomChanged();
}

void QuickDolphinController::zoomOut()
{
    m_zoom->zoomOut();
    emit zoomChanged();
}

void QuickDolphinController::resetZoom()
{
    m_zoom->reset();
    emit zoomChanged();
}

void QuickDolphinController::fitScreen(qreal viewportWidth, qreal viewportHeight, qreal headerWidth, qreal headerHeight)
{
    if (!m_model) {
        return;
    }
    m_zoom->fitScreen(viewportWidth, viewportHeight, headerWidth, headerHeight,
                      m_model->columnCount(), m_model->rowCount());
    emit zoomChanged();
}

void QuickDolphinController::applyToSelectedCells(const std::function<int(int)> &valueFn)
{
    const auto indexes = m_selectionModel->selectedIndexes();
    const auto [cells, oldValues] = snapshotCells(*m_model, indexes);

    DolphinEdits::applyToCells(*m_model, indexes, valueFn);

    m_undoStack.push(new SetCellsCommand(m_model, cells, oldValues, [this] { m_edited = true; run(); }));
}

void QuickDolphinController::setSelectedTo0()
{
    applyToSelectedCells([](int) { return 0; });
}

void QuickDolphinController::setSelectedTo1()
{
    applyToSelectedCells([](int) { return 1; });
}

void QuickDolphinController::invertSelected()
{
    applyToSelectedCells([](int v) { return (v + 1) % 2; });
}

void QuickDolphinController::clearInputs()
{
    const auto [cells, oldValues] = snapshotCells(*m_model, allCellIndexes(*m_model, m_inputPorts, m_model->columnCount()));

    DolphinEdits::clearInputs(*m_model, m_inputPorts);

    m_undoStack.push(new SetCellsCommand(m_model, cells, oldValues, [this] { m_edited = true; run(); }));
}

void QuickDolphinController::combinational()
{
    const int truthTableSize = static_cast<int>((std::min)(static_cast<double>(SignalModel::kMaxColumns), std::pow(2, m_inputPorts)));
    setLengthInternal(truthTableSize, false);

    const auto [cells, oldValues] = snapshotCells(*m_model, allCellIndexes(*m_model, m_inputPorts, m_model->columnCount()));

    DolphinEdits::combinational(*m_model, m_inputPorts, m_model->columnCount());

    m_undoStack.push(new SetCellsCommand(m_model, cells, oldValues, [this] { m_edited = true; run(); }));
}

void QuickDolphinController::autoCrop()
{
    const int lastNonZero = DolphinEdits::lastNonZeroColumn(*m_model, m_inputPorts);
    setLengthInternal(lastNonZero + 1, true);
}

void QuickDolphinController::applyClockWave(int period)
{
    const auto ranges = m_selectionModel->selection();
    // Anchor the clock phase to the leftmost selected column so the waveform starts at 0
    // regardless of where in the timeline the selection begins.
    const int firstCol = DolphinClipboard::firstColumn(*m_model, ranges);

    const auto indexes = m_selectionModel->selectedIndexes();
    const auto [cells, oldValues] = snapshotCells(*m_model, indexes);

    DolphinEdits::clockWave(*m_model, indexes, firstCol, period);

    m_undoStack.push(new SetCellsCommand(m_model, cells, oldValues, [this] { m_edited = true; run(); }));
}

void QuickDolphinController::setLength(int newLength)
{
    setLengthInternal(newLength, true);
}

void QuickDolphinController::setLengthInternal(int newLength, bool runSimulation)
{
    if (m_length == newLength) {
        return;
    }

    m_length = newLength;

    if (newLength <= m_model->columnCount()) {
        // Shrinking: Qt's setColumnCount removes trailing columns automatically.
        m_model->setColumnCount(newLength);
        m_edited = true;
        emit modelChanged();
        return;
    }

    // Growing: new input columns must be explicitly filled with zeros; output columns are
    // populated by run() and don't need pre-filling.
    const int oldLength = m_model->columnCount();
    m_model->setColumnCount(newLength);
    DolphinEdits::growInputColumns(*m_model, m_inputPorts, oldLength, newLength);

    m_edited = true;
    if (runSimulation) {
        run();
    }
    emit modelChanged();
}

void QuickDolphinController::cut()
{
    const auto ranges = m_selectionModel->selection();
    if (ranges.isEmpty()) {
        QGuiApplication::clipboard()->clear();
        return;
    }

    // Cut = copy the selection, then clear it (which re-runs the simulation, pushes undo, and
    // already sets m_edited itself -- set here too, redundant but explicit, matching
    // BewavedDolphin::on_actionCut_triggered()'s own identical redundant set).
    DolphinClipboard::copyToClipboard(*m_model, ranges);
    setSelectedTo0();
    m_edited = true;
}

void QuickDolphinController::copy()
{
    const auto ranges = m_selectionModel->selection();
    if (ranges.isEmpty()) {
        QGuiApplication::clipboard()->clear();
        return;
    }

    DolphinClipboard::copyToClipboard(*m_model, ranges);
}

void QuickDolphinController::paste()
{
    const auto ranges = m_selectionModel->selection();
    if (ranges.isEmpty()) {
        return;
    }

    if (DolphinClipboard::pasteFromClipboard(*m_model, ranges)) {
        m_edited = true;
        run();
    }
}

void QuickDolphinController::showStatusMessage(const QString &message, int timeout)
{
    m_statusMessage = message;
    emit statusMessageChanged();
    m_statusMessageTimer.start(timeout);
}

void QuickDolphinController::save()
{
    if (m_currentFile.fileName().isEmpty()) {
        saveAs();
        return;
    }

    DolphinFile::save(*m_model, m_currentFile.absoluteFilePath(), m_inputPorts);
    showStatusMessage(tr("Saved file successfully."), 4000);
    m_edited = false;
}

void QuickDolphinController::saveAs()
{
    // List the format that matches the current file first so it is the default selection.
    const QString fileFilter = m_currentFile.fileName().endsWith(".csv")
        ? tr("CSV files") + " (*.csv);;" + tr("Dolphin files") + " (*.dolphin);;" + tr("All supported files") + " (*.dolphin *.csv)"
        : tr("Dolphin files") + " (*.dolphin);;" + tr("CSV files") + " (*.csv);;" + tr("All supported files") + " (*.dolphin *.csv)";

    const auto result = FileDialogs::provider()->getSaveFileName(nullptr, tr("Save File as..."), m_currentFile.absoluteFilePath(), fileFilter);
    QString fileName = result.fileName;
    if (fileName.isEmpty()) {
        return;
    }

    // Append the correct extension when the user types a bare name without one, inferring the
    // format from whichever filter was active in the dialog.
    if (!fileName.endsWith(".dolphin") && !fileName.endsWith(".csv")) {
        fileName.append(result.selectedFilter.contains("dolphin") ? ".dolphin" : ".csv");
    }

    DolphinFile::save(*m_model, fileName, m_inputPorts);
    m_currentFile = QFileInfo(fileName);
    emit currentFileChanged();
    associateToWiRedPanda(fileName);
    showStatusMessage(tr("Saved file successfully."), 4000);
    m_edited = false;
}

void QuickDolphinController::load()
{
    // Prefer the last-used dolphin file's directory; fall back to the host's current directory,
    // then the home directory -- mirrors BewavedDolphin::on_actionLoad_triggered()'s own
    // fallback chain (m_currentFile -> m_host->currentFile() -> QDir::homePath()) now that a
    // real DolphinHost exists (Phase 6e).
    QString defaultDir;
    if (m_currentFile.exists()) {
        defaultDir = m_currentFile.absolutePath();
    } else if (m_host && m_host->currentFile().exists()) {
        defaultDir = m_host->currentDir().absolutePath();
    } else {
        defaultDir = QDir::homePath();
    }

    const QString fileName = FileDialogs::provider()->getOpenFileName(
        nullptr, tr("Open File"), defaultDir,
        tr("All supported files") + " (*.dolphin *.csv);;" + tr("Dolphin files") + " (*.dolphin);;" + tr("CSV files") + " (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }

    loadFile(fileName);
    m_edited = false;
    showStatusMessage(tr("File loaded successfully."), 4000);
}

void QuickDolphinController::loadFile(const QString &fileName)
{
    applyWaveformData(DolphinFile::load(fileName, m_inputPorts));
    m_currentFile = QFileInfo(fileName);
    emit currentFileChanged();
    associateToWiRedPanda(fileName);
}

void QuickDolphinController::applyWaveformData(const DolphinSerializer::WaveformData &fileData)
{
    setLengthInternal(fileData.columns, false);

    {
        SignalModel::BulkEditGuard guard(*m_model);
        for (int row = 0; row < fileData.inputPorts; ++row) {
            for (int col = 0; col < fileData.columns; ++col) {
                m_model->setValue(row, col, fileData.values[row * fileData.columns + col]);
            }
        }
    }

    run();
}

void QuickDolphinController::exportToPng()
{
    QString pngFile = FileDialogs::provider()->getSaveFileName(nullptr, tr("Export to Image"), m_currentFile.absolutePath(), tr("PNG files") + " (*.png)").fileName;
    if (pngFile.isEmpty()) {
        return;
    }
    if (!pngFile.endsWith(".png", Qt::CaseInsensitive)) {
        pngFile.append(".png");
    }

    QuickDolphinExporter::exportToPng(m_model, m_numberMode ? PlotType::Number : PlotType::Line, pngFile);
}

void QuickDolphinController::exportToPdf()
{
    QString pdfFile = FileDialogs::provider()->getSaveFileName(nullptr, tr("Export to PDF"), m_currentFile.absolutePath(), tr("PDF files") + " (*.pdf)").fileName;
    if (pdfFile.isEmpty()) {
        return;
    }
    if (!pdfFile.endsWith(".pdf", Qt::CaseInsensitive)) {
        pdfFile.append(".pdf");
    }

    QuickDolphinExporter::exportToPdf(m_model, m_numberMode ? PlotType::Number : PlotType::Line, pdfFile);
}

void QuickDolphinController::associateToWiRedPanda(const QString &fileName)
{
    // Only prompt when the file is new (not already linked) and we are in interactive mode --
    // non-interactive (command-line / test / MCP) sessions skip the dialog entirely.
    if (m_host && m_host->dolphinFileName() != fileName && Application::interactiveMode) {
        const DialogButton reply = Dialogs::provider()->choice(
            tr("wiRedPanda - beWavedDolphin"),
            tr("Do you want to link this beWavedDolphin file to your current wiRedPanda file and save it?"),
            {DialogButton::Yes, DialogButton::No}, DialogButton::No);

        if (reply == DialogButton::Yes) {
            m_host->setDolphinFileName(fileName);
            m_host->save({});
        }
    }
}

bool QuickDolphinController::checkSave()
{
    if (!m_edited) {
        return true;
    }

    const DialogButton reply = Dialogs::provider()->choice(
        tr("wiRedPanda - beWavedDolphin"), tr("Save simulation before closing?"),
        {DialogButton::Yes, DialogButton::No, DialogButton::Cancel}, DialogButton::Cancel);

    switch (reply) {
    // After saving, check m_edited again: if the save itself failed (e.g. the user dismissed
    // the Save As dialog), m_edited stays true and the close should not be allowed.
    case DialogButton::Yes: save(); return !m_edited;
    case DialogButton::No: return true;
    case DialogButton::Cancel: return false;
    default: return true;
    }
}

bool QuickDolphinController::elementsStillLive() const
{
    if (!m_canvas) {
        return false;
    }
    const auto canvasElements = m_canvas->elements();
    for (auto *elm : std::as_const(m_inputs)) {
        if (!canvasElements.contains(elm)) {
            return false;
        }
    }
    for (auto *elm : std::as_const(m_outputs)) {
        if (!canvasElements.contains(elm)) {
            return false;
        }
    }
    return true;
}

int QuickDolphinController::inputRow(const QString &label) const
{
    for (int row = 0; row < m_inputs.size(); ++row) {
        if (m_inputs.at(row)->label() == label) {
            return row;
        }
    }
    return -1;
}

QuickDolphinController::WaveformSnapshot QuickDolphinController::snapshot(int duration) const
{
    // Read the computed waveform into a plain DTO so automation consumers (the MCP server) need
    // neither the live model nor the element vectors. Input rows come first; output rows follow
    // at the m_inputs.size() offset (mirrors the table's own row layout).
    WaveformSnapshot result;

    for (int row = 0; row < m_inputs.size(); ++row) {
        Signal signal;
        signal.label = m_inputs.at(row)->label();
        for (int col = 0; col < duration; ++col) {
            signal.values.append(m_model->value(row, col));
        }
        result.inputs.append(signal);
    }

    for (int row = 0; row < m_outputs.size(); ++row) {
        Signal signal;
        signal.label = m_outputs.at(row)->label();
        const int outputRowIndex = static_cast<int>(m_inputs.size()) + row;
        for (int col = 0; col < duration; ++col) {
            signal.values.append(m_model->value(outputRowIndex, col));
        }
        result.outputs.append(signal);
    }

    return result;
}

void QuickDolphinController::saveToTxt(QTextStream &stream) const
{
    if (!m_model) {
        return;
    }

    // Same staleness hazard as run() (see elementsStillLive()); this dereferences m_inputs/
    // m_outputs via sweep() below too. Throw instead of silently no-op'ing so an MCP client
    // (the only realistic caller once run() already guards interactive use) gets a clean error
    // instead of a "successful" empty/truncated export.
    if (!elementsStillLive()) {
        throw PANDACEPTION("Cannot export: the circuit this waveform was built from has changed.");
    }

    // Dump the full combinational truth table into a throwaway model (like exportToPng()'s own
    // renderToImage() uses a throwaway rendering path) so the live document is never mutated --
    // exporting must not clobber the user's waveform (e.g. an MCP persistent session). Clamp to
    // SignalModel::kMaxColumns, exactly like combinational()'s identical cap -- otherwise a
    // circuit with ~25+ effective input ports allocates tens of millions of cells, and
    // m_inputPorts >= 31 overflows the int cast (UB).
    const int columns = static_cast<int>((std::min)(static_cast<double>(SignalModel::kMaxColumns), std::pow(2, m_inputPorts)));
    SignalModel truthTable(m_model->rowCount(), columns);

    QStringList labels;
    labels.reserve(m_model->rowCount());
    for (int row = 0; row < m_model->rowCount(); ++row) {
        labels.append(m_model->verticalHeaderItem(row)->text());
    }
    truthTable.setVerticalHeaderLabels(labels);

    DolphinEdits::combinational(truthTable, m_inputPorts, columns);

    // Compute the outputs for every input combination, then restore the live inputs the sweep
    // perturbed (the same capture/restore contract run() relies on).
    const QVector<Status> oldInputValues = WaveformSimulator::captureInputs(m_inputs, m_inputPorts);
    {
        SignalModel::BulkEditGuard guard(truthTable);
        m_simDriver->sweep(
            m_inputs, m_outputs, m_canvas->elements(), m_inputPorts, columns,
            [&truthTable](int row, int col) { return truthTable.value(row, col) != 0; },
            [&truthTable](int row, int col, int value) { truthTable.setValue(row, col, value); });
    }
    WaveformSimulator::restoreInputs(m_inputs, oldInputValues);

    DolphinExporter::writeTruthTableText(stream, &truthTable, static_cast<int>(m_inputs.size()));
}
